/****************************************************************************
 * apps/examples/aw8697demo/aw8697demo_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <nuttx/bits.h>
#include <nuttx/input/aw8697.h>
#include <nuttx/input/ff.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_AW8697DEMO_DEVPATH
#  define CONFIG_EXAMPLES_AW8697DEMO_DEVPATH "/dev/input0"
#endif

#ifndef CONFIG_EXAMPLES_AW8697DEMO_DURATION_MS
#  define CONFIG_EXAMPLES_AW8697DEMO_DURATION_MS 120
#endif

#ifndef CONFIG_EXAMPLES_AW8697DEMO_STRENGTH
#  define CONFIG_EXAMPLES_AW8697DEMO_STRENGTH 32767
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void aw8697demo_usage(FAR const char *progname)
{
  printf("Usage: %s [-d devpath] [-t ms] [-s strength]\n", progname);
  printf("  -d devpath   default: %s\n", CONFIG_EXAMPLES_AW8697DEMO_DEVPATH);
  printf("  -t ms        default: %d\n",
         CONFIG_EXAMPLES_AW8697DEMO_DURATION_MS);
  printf("  -s strength  1..65535, default: %d\n",
         CONFIG_EXAMPLES_AW8697DEMO_STRENGTH);
}

static int aw8697demo_parse_u32(FAR const char *arg, FAR uint32_t *value)
{
  FAR char *endptr;
  unsigned long tmp;

  errno = 0;
  tmp = strtoul(arg, &endptr, 0);
  if (errno != 0 || *endptr != '\0')
    {
      return -EINVAL;
    }

  *value = tmp;
  return OK;
}

static int aw8697demo_play(FAR const char *devpath, uint32_t duration_ms,
                           uint32_t strength)
{
  unsigned long features[BITS_TO_LONGS(FF_CNT)];
  struct ff_effect effect;
  struct ff_event_s event;
  uint8_t chip_id;
  int fd;
  int ret = OK;

  fd = open(devpath, O_RDWR);
  if (fd < 0)
    {
      printf("aw8697demo: open %s failed: %d\n", devpath, errno);
      return -errno;
    }

  ret = ioctl(fd, AW8697IOC_READ_ID, &chip_id);
  if (ret < 0)
    {
      printf("aw8697demo: read chip id failed: %d\n", errno);
      ret = -errno;
      goto out_close;
    }

  printf("AW8697 chip id: 0x%02x\n", chip_id);

  memset(features, 0, sizeof(features));
  ret = ioctl(fd, EVIOCGBIT, features);
  if (ret < 0)
    {
      printf("aw8697demo: query ff features failed: %d\n", errno);
      ret = -errno;
      goto out_close;
    }

  if (!test_bit(FF_RUMBLE, features))
    {
      printf("aw8697demo: device does not report FF_RUMBLE\n");
      ret = -ENOTSUP;
      goto out_close;
    }

  memset(&effect, 0, sizeof(effect));
  effect.type = FF_RUMBLE;
  effect.id = -1;
  effect.replay.length = duration_ms;
  effect.u.rumble.strong_magnitude = strength;
  effect.u.rumble.weak_magnitude = strength / 2;

  ret = ioctl(fd, EVIOCSFF, &effect);
  if (ret < 0)
    {
      printf("aw8697demo: upload effect failed: %d\n", errno);
      ret = -errno;
      goto out_close;
    }

  event.code = FF_GAIN;
  event.value = 0xffff;
  if (write(fd, &event, sizeof(event)) != sizeof(event))
    {
      printf("aw8697demo: set gain failed: %d\n", errno);
      ret = -errno;
      goto out_erase;
    }

  printf("AW8697 vibrate: id=%d duration=%lu ms strength=%lu\n",
         effect.id, (unsigned long)duration_ms, (unsigned long)strength);

  event.code = effect.id;
  event.value = 1;
  if (write(fd, &event, sizeof(event)) != sizeof(event))
    {
      printf("aw8697demo: start effect failed: %d\n", errno);
      ret = -errno;
      goto out_erase;
    }

  usleep((duration_ms + 50) * 1000);

  event.code = effect.id;
  event.value = 0;
  (void)write(fd, &event, sizeof(event));

out_erase:
  (void)ioctl(fd, EVIOCRMFF, effect.id);

out_close:
  close(fd);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  FAR const char *devpath = CONFIG_EXAMPLES_AW8697DEMO_DEVPATH;
  uint32_t duration_ms = CONFIG_EXAMPLES_AW8697DEMO_DURATION_MS;
  uint32_t strength = CONFIG_EXAMPLES_AW8697DEMO_STRENGTH;
  uint32_t value;
  int option;
  int ret;

  while ((option = getopt(argc, argv, "d:hs:t:")) != ERROR)
    {
      switch (option)
        {
          case 'd':
            devpath = optarg;
            break;

          case 's':
            ret = aw8697demo_parse_u32(optarg, &value);
            if (ret < 0 || value == 0 || value > 0xffff)
              {
                aw8697demo_usage(argv[0]);
                return EXIT_FAILURE;
              }

            strength = value;
            break;

          case 't':
            ret = aw8697demo_parse_u32(optarg, &value);
            if (ret < 0 || value == 0 || value > 0x7fff)
              {
                aw8697demo_usage(argv[0]);
                return EXIT_FAILURE;
              }

            duration_ms = value;
            break;

          case 'h':
          default:
            aw8697demo_usage(argv[0]);
            return option == 'h' ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }

  ret = aw8697demo_play(devpath, duration_ms, strength);
  return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
