/****************************************************************************
 * apps/examples/bme280_dmo/bme280_dmo_main.c
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
#include <nuttx/sensors/bme280.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_BME280_DMO_DEVPATH
#  define CONFIG_EXAMPLES_BME280_DMO_DEVPATH "/dev/bme280"
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void bme280_dmo_print_signed_centi(FAR const char *label,
                                          int32_t value)
{
  FAR const char *sign = "";

  if (value < 0)
    {
      sign = "-";
      value = -value;
    }

  printf("%s: %s%" PRId32 ".%02" PRId32 " C\n",
         label, sign, value / 100, value % 100);
}

static void bme280_dmo_print_humidity(uint32_t humidity)
{
  uint32_t milli;

  milli = ((uint64_t)humidity * 1000u) / 1024u;
  printf("Humidity: %" PRIu32 ".%03" PRIu32 " %%RH\n",
         milli / 1000u, milli % 1000u);
}

static void bme280_dmo_print_pressure(uint32_t pressure)
{
  uint32_t milli_hpa;

  milli_hpa = ((uint64_t)pressure * 1000u) / (256u * 100u);
  printf("Pressure: %" PRIu32 ".%03" PRIu32 " hPa\n",
         milli_hpa / 1000u, milli_hpa % 1000u);
}

static int bme280_dmo_print_sample(int fd)
{
  struct bme280_sample_s sample;
  ssize_t nread;

  nread = read(fd, &sample, sizeof(sample));
  if (nread != sizeof(sample))
    {
      printf("bme280_demo: read sample failed: %d\n", errno);
      return ERROR;
    }

  bme280_dmo_print_signed_centi("Temperature", sample.temperature);
  bme280_dmo_print_humidity(sample.humidity);
  bme280_dmo_print_pressure(sample.pressure);
  putchar('\n');
  fflush(stdout);

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *devpath = CONFIG_EXAMPLES_BME280_DMO_DEVPATH;
  uint8_t chip_id;
  int fd;
  int ret;

  fd = open(devpath, O_RDONLY);
  if (fd < 0)
    {
      printf("bme280_demo: open %s failed: %d\n", devpath, errno);
      return EXIT_FAILURE;
    }

  ret = ioctl(fd, BME280IOC_READ_ID, &chip_id);
  if (ret < 0)
    {
      printf("bme280_demo: read chip id failed: %d\n", errno);
      close(fd);
      return EXIT_FAILURE;
    }

  printf("BME280 chip id: 0x%02x\n", chip_id);

  for (; ; )
    {
      ret = bme280_dmo_print_sample(fd);
      if (ret < 0)
        {
          close(fd);
          return EXIT_FAILURE;
        }

      sleep(1);
    }
}
