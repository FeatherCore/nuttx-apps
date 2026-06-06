/****************************************************************************
 * apps/examples/lsm6dsox_demo/lsm6dsox_demo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <nuttx/sensors/lsm6dsox.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_LSM6DSOX_DEMO_DEVPATH
#  define CONFIG_EXAMPLES_LSM6DSOX_DEMO_DEVPATH "/dev/lsm6dsox"
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void lsm6dsox_demo_print_fixed(FAR const char *label, int32_t value,
                                      int digits, FAR const char *unit)
{
  int32_t scale = 1;
  int32_t whole;
  int32_t frac;
  int i;

  for (i = 0; i < digits; i++)
    {
      scale *= 10;
    }

  if (value < 0)
    {
      printf("%s: -", label);
      value = -value;
    }
  else
    {
      printf("%s: ", label);
    }

  whole = value / scale;
  frac = value % scale;

  printf("%" PRId32, whole);
  if (digits > 0)
    {
      printf(".%0*" PRId32, digits, frac);
    }

  printf(" %s\n", unit);
}

static int32_t lsm6dsox_demo_accel_mg_x10(int16_t raw)
{
  return ((int32_t)raw * 61) / 100;
}

static int32_t lsm6dsox_demo_gyro_dps_x100(int16_t raw)
{
  return ((int32_t)raw * 875) / 1000;
}

static int32_t lsm6dsox_demo_temp_c_x100(int16_t raw)
{
  return 2500 + (((int32_t)raw * 100) / 256);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *devpath = CONFIG_EXAMPLES_LSM6DSOX_DEMO_DEVPATH;
  struct lsm6dsox_sample_s sample;
  struct lsm6dsox_id_s id;
  ssize_t nread;
  int fd;
  int ret;

  fd = open(devpath, O_RDONLY);
  if (fd < 0)
    {
      printf("lsm6dsox_demo: open %s failed: %d\n", devpath, errno);
      return EXIT_FAILURE;
    }

  ret = ioctl(fd, LSM6DSOXIOC_READ_ID, &id);
  if (ret < 0)
    {
      printf("lsm6dsox_demo: read chip id failed: %d\n", errno);
      close(fd);
      return EXIT_FAILURE;
    }

  printf("LSM6DSOX WHO_AM_I: 0x%02x\n", id.whoami);

  for (; ; )
    {
      nread = read(fd, &sample, sizeof(sample));
      if (nread != sizeof(sample))
        {
          printf("lsm6dsox_demo: read sample failed: %d\n", errno);
          close(fd);
          return EXIT_FAILURE;
        }

      printf("Raw accel: x=%d y=%d z=%d\n",
             sample.accel_x, sample.accel_y, sample.accel_z);
      lsm6dsox_demo_print_fixed("Accel X",
                                lsm6dsox_demo_accel_mg_x10(
                                  sample.accel_x),
                                1, "mg");
      lsm6dsox_demo_print_fixed("Accel Y",
                                lsm6dsox_demo_accel_mg_x10(
                                  sample.accel_y),
                                1, "mg");
      lsm6dsox_demo_print_fixed("Accel Z",
                                lsm6dsox_demo_accel_mg_x10(
                                  sample.accel_z),
                                1, "mg");

      printf("Raw gyro: x=%d y=%d z=%d\n",
             sample.gyro_x, sample.gyro_y, sample.gyro_z);
      lsm6dsox_demo_print_fixed("Gyro X",
                                lsm6dsox_demo_gyro_dps_x100(
                                  sample.gyro_x),
                                2, "dps");
      lsm6dsox_demo_print_fixed("Gyro Y",
                                lsm6dsox_demo_gyro_dps_x100(
                                  sample.gyro_y),
                                2, "dps");
      lsm6dsox_demo_print_fixed("Gyro Z",
                                lsm6dsox_demo_gyro_dps_x100(
                                  sample.gyro_z),
                                2, "dps");

      printf("Raw temperature: %d\n", sample.temperature);
      lsm6dsox_demo_print_fixed("Temperature",
                                lsm6dsox_demo_temp_c_x100(
                                  sample.temperature),
                                2, "C");
      printf("\n");

      sleep(1);
    }

  close(fd);
  return EXIT_SUCCESS;
}
