/****************************************************************************
 * apps/examples/max30102_demo/max30102_demo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/ioctl.h>

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <nuttx/sensors/max30102.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_MAX30102_DEMO_DEVPATH
#  define CONFIG_EXAMPLES_MAX30102_DEMO_DEVPATH "/dev/max30102"
#endif

#define MAX30102_DEMO_SAMPLE_RATE       100
#define MAX30102_DEMO_WINDOW_SAMPLES    400
#define MAX30102_DEMO_MIN_IR            5000
#define MAX30102_DEMO_MIN_AC            100
#define MAX30102_DEMO_REFRACTORY        30

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct max30102_demo_result_s
{
  bool finger;
  bool heart_rate_valid;
  bool spo2_valid;
  int32_t heart_rate;
  int32_t spo2;
  uint32_t red_mean;
  uint32_t ir_mean;
  uint32_t red_ac;
  uint32_t ir_ac;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct max30102_sample_s
  g_samples[MAX30102_DEMO_WINDOW_SAMPLES];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t max30102_demo_div_u64(uint64_t value, uint64_t divisor)
{
  if (divisor == 0)
    {
      return 0;
    }

  return (uint32_t)(value / divisor);
}

static void max30102_demo_collect_stats(unsigned int count,
                                        unsigned int start,
                                        FAR struct max30102_demo_result_s *r)
{
  uint64_t red_sum = 0;
  uint64_t ir_sum = 0;
  uint32_t red_min = UINT32_MAX;
  uint32_t red_max = 0;
  uint32_t ir_min = UINT32_MAX;
  uint32_t ir_max = 0;
  uint32_t red;
  uint32_t ir;
  unsigned int i;
  unsigned int index;

  for (i = 0; i < count; i++)
    {
      index = (start + i) % MAX30102_DEMO_WINDOW_SAMPLES;
      red = g_samples[index].red;
      ir = g_samples[index].ir;

      red_sum += red;
      ir_sum += ir;

      if (red < red_min)
        {
          red_min = red;
        }

      if (red > red_max)
        {
          red_max = red;
        }

      if (ir < ir_min)
        {
          ir_min = ir;
        }

      if (ir > ir_max)
        {
          ir_max = ir;
        }
    }

  r->red_mean = max30102_demo_div_u64(red_sum, count);
  r->ir_mean = max30102_demo_div_u64(ir_sum, count);
  r->red_ac = red_max - red_min;
  r->ir_ac = ir_max - ir_min;
  r->finger = r->ir_mean >= MAX30102_DEMO_MIN_IR &&
              r->ir_ac >= MAX30102_DEMO_MIN_AC;
}

static void max30102_demo_estimate_spo2(FAR struct max30102_demo_result_s *r)
{
  uint32_t ratio;
  int32_t spo2;

  if (!r->finger || r->red_mean == 0 || r->ir_mean == 0 ||
      r->red_ac == 0 || r->ir_ac == 0)
    {
      return;
    }

  ratio = max30102_demo_div_u64((uint64_t)r->red_ac * r->ir_mean * 1000u,
                                (uint64_t)r->ir_ac * r->red_mean);
  spo2 = 1100 - (int32_t)(ratio / 4);

  if (spo2 > 1000)
    {
      spo2 = 1000;
    }
  else if (spo2 < 0)
    {
      spo2 = 0;
    }

  r->spo2 = spo2;
  r->spo2_valid = true;
}

static void max30102_demo_estimate_heart_rate(unsigned int count,
                                              unsigned int start,
                                              FAR struct
                                              max30102_demo_result_s *r)
{
  int peak_first = -1;
  int peak_last = -1;
  int peak_count = 0;
  int last_peak = -MAX30102_DEMO_REFRACTORY;
  uint32_t threshold;
  uint32_t previous;
  uint32_t current;
  uint32_t next;
  unsigned int i;
  unsigned int index;
  unsigned int prev_index;
  unsigned int next_index;

  if (!r->finger || count < MAX30102_DEMO_SAMPLE_RATE * 2)
    {
      return;
    }

  threshold = r->ir_mean + (r->ir_ac / 4);

  for (i = 1; i + 1 < count; i++)
    {
      prev_index = (start + i - 1) % MAX30102_DEMO_WINDOW_SAMPLES;
      index = (start + i) % MAX30102_DEMO_WINDOW_SAMPLES;
      next_index = (start + i + 1) % MAX30102_DEMO_WINDOW_SAMPLES;

      previous = g_samples[prev_index].ir;
      current = g_samples[index].ir;
      next = g_samples[next_index].ir;

      if (current > previous && current >= next && current > threshold &&
          (int)i - last_peak >= MAX30102_DEMO_REFRACTORY)
        {
          if (peak_first < 0)
            {
              peak_first = i;
            }

          peak_last = i;
          last_peak = i;
          peak_count++;
        }
    }

  if (peak_count >= 2 && peak_last > peak_first)
    {
      r->heart_rate =
        (600 * MAX30102_DEMO_SAMPLE_RATE * (peak_count - 1)) /
        (peak_last - peak_first);
      r->heart_rate_valid = true;
    }
}

static void max30102_demo_analyze(unsigned int total,
                                  FAR struct max30102_demo_result_s *r)
{
  unsigned int count;
  unsigned int start;

  count = total < MAX30102_DEMO_WINDOW_SAMPLES ?
          total : MAX30102_DEMO_WINDOW_SAMPLES;
  start = total < MAX30102_DEMO_WINDOW_SAMPLES ?
          0 : total % MAX30102_DEMO_WINDOW_SAMPLES;

  r->finger = false;
  r->heart_rate_valid = false;
  r->spo2_valid = false;
  r->heart_rate = 0;
  r->spo2 = 0;

  max30102_demo_collect_stats(count, start, r);
  max30102_demo_estimate_spo2(r);
  max30102_demo_estimate_heart_rate(count, start, r);
}

static void max30102_demo_print_fixed_tenth(FAR const char *label,
                                            int32_t value,
                                            FAR const char *unit)
{
  printf("%s: %" PRId32 ".%01" PRId32 " %s\n",
         label, value / 10, value % 10, unit);
}

static void max30102_demo_print_result(unsigned int total,
                                       FAR const struct
                                       max30102_demo_result_s *r,
                                       FAR const struct
                                       max30102_sample_s *last)
{
  if (total < MAX30102_DEMO_SAMPLE_RATE * 2)
    {
      printf("Collecting samples: %u/%u\n",
             total, MAX30102_DEMO_SAMPLE_RATE * 2);
    }
  else if (!r->finger)
    {
      printf("Finger: not detected\n");
    }
  else
    {
      if (r->heart_rate_valid)
        {
          max30102_demo_print_fixed_tenth("Heart rate",
                                          r->heart_rate, "bpm");
        }
      else
        {
          printf("Heart rate: -- bpm\n");
        }

      if (r->spo2_valid)
        {
          max30102_demo_print_fixed_tenth("SpO2", r->spo2, "%");
        }
      else
        {
          printf("SpO2: -- %%\n");
        }
    }

  printf("Raw: red=%" PRIu32 " ir=%" PRIu32
         " dc_red=%" PRIu32 " dc_ir=%" PRIu32 "\n\n",
         last->red, last->ir, r->red_mean, r->ir_mean);
  fflush(stdout);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *devpath = CONFIG_EXAMPLES_MAX30102_DEMO_DEVPATH;
  struct max30102_demo_result_s result;
  struct max30102_sample_s sample;
  struct max30102_id_s id;
  unsigned int total = 0;
  ssize_t nread;
  int fd;
  int ret;

  fd = open(devpath, O_RDONLY);
  if (fd < 0)
    {
      printf("max30102_demo: open %s failed: %d\n", devpath, errno);
      return EXIT_FAILURE;
    }

  ret = ioctl(fd, MAX30102IOC_READ_ID, &id);
  if (ret < 0)
    {
      printf("max30102_demo: read chip id failed: %d\n", errno);
      close(fd);
      return EXIT_FAILURE;
    }

  printf("MAX30102 part id: 0x%02x revision: 0x%02x\n",
         id.part_id, id.revision_id);

  for (; ; )
    {
      nread = read(fd, &sample, sizeof(sample));
      if (nread != sizeof(sample))
        {
          printf("max30102_demo: read sample failed: %d\n", errno);
          close(fd);
          return EXIT_FAILURE;
        }

      g_samples[total % MAX30102_DEMO_WINDOW_SAMPLES] = sample;
      total++;

      if ((total % MAX30102_DEMO_SAMPLE_RATE) == 0)
        {
          max30102_demo_analyze(total, &result);
          max30102_demo_print_result(total, &result, &sample);
        }
    }
}
