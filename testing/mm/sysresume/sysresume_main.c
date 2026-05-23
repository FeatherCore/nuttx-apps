/****************************************************************************
 * apps/testing/mm/sysresume/sysresume_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <malloc.h>
#include <sched.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SYSRESUME_INTSRAM_BASE  0x34000000ul
#define SYSRESUME_INTSRAM_END   0x34200000ul
#define SYSRESUME_PSRAM_BASE    0x90000000ul
#define SYSRESUME_PSRAM_END     0x92000000ul

#define SYSRESUME_BUFSIZE       160

#ifndef CONFIG_TESTING_SYSCALL_RESUME_DELAY_USEC
#  define CONFIG_TESTING_SYSCALL_RESUME_DELAY_USEC 5000000
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sysresume_options_s
{
  int loops;
  int child_priority;
  useconds_t delay_usec;
  bool child_task;
  bool mallinfo_after_sleep;
  bool wait_child;
  bool yield_after_notify;
  bool sleep_after_notify;
  bool direct_exit_after_notify;
  bool child_priority_set;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct sysresume_options_s g_child_options;
static sem_t g_child_done;
static volatile int g_child_status;
static volatile bool g_child_notify;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static FAR const char *sysresume_region(FAR const void *ptr)
{
  uintptr_t addr = (uintptr_t)ptr;

  if (addr >= SYSRESUME_PSRAM_BASE && addr < SYSRESUME_PSRAM_END)
    {
      return "psram";
    }

  if (addr >= SYSRESUME_INTSRAM_BASE && addr < SYSRESUME_INTSRAM_END)
    {
      return "internal";
    }

  return "other";
}

static void sysresume_write_all(FAR const char *msg, size_t len)
{
  ssize_t nwritten;

  while (len > 0)
    {
      nwritten = write(1, msg, len);
      if (nwritten <= 0)
        {
          return;
        }

      msg += nwritten;
      len -= nwritten;
    }
}

static void sysresume_puts(FAR const char *msg)
{
  sysresume_write_all(msg, strlen(msg));
}

static void sysresume_puthex32(uint32_t value)
{
  static const char hexdigits[] = "0123456789abcdef";
  char text[8];
  int i;

  for (i = 7; i >= 0; i--)
    {
      text[i] = hexdigits[value & 0x0f];
      value >>= 4;
    }

  sysresume_write_all(text, sizeof(text));
}

static void sysresume_printf(FAR const char *fmt, ...)
{
  char buffer[SYSRESUME_BUFSIZE];
  va_list ap;
  int len;

  va_start(ap, fmt);
  len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);

  if (len < 0)
    {
      return;
    }

  if (len >= sizeof(buffer))
    {
      len = sizeof(buffer) - 1;
    }

  sysresume_write_all(buffer, len);
}

static void sysresume_print_mallinfo(FAR const char *prefix, int ret,
                                     FAR const struct mallinfo *info)
{
  sysresume_puts(prefix);
  sysresume_puts(" ret=");
  sysresume_puthex32((uint32_t)ret);
  sysresume_puts(" arena=");
  sysresume_puthex32((uint32_t)info->arena);
  sysresume_puts(" uordblks=");
  sysresume_puthex32((uint32_t)info->uordblks);
  sysresume_puts(" fordblks=");
  sysresume_puthex32((uint32_t)info->fordblks);
  sysresume_puts("\n");
}

static int sysresume_run(FAR const char *name,
                         FAR const struct sysresume_options_s *options)
{
  volatile uint32_t local = 0x53595245;
  struct mallinfo info;
  int ret;
  int i;

  sysresume_printf("sysresume: %s pid=%d local=%p region=%s loops=%d "
                   "delay=%lu\n",
                   name, getpid(), &local, sysresume_region((FAR void *)&local),
                   options->loops, (unsigned long)options->delay_usec);

  for (i = 0; i < options->loops; i++)
    {
      sysresume_printf("sysresume: %s loop=%d before-write\n", name, i);
      sysresume_puts("sysresume: marker before sleep\n");

      errno = 0;
      ret = usleep(options->delay_usec);
      sysresume_printf("sysresume: %s loop=%d after-usleep ret=%d errno=%d\n",
                       name, i, ret, errno);

      if (options->mallinfo_after_sleep)
        {
          sysresume_puts("sysresume: before mallinfo\n");
          info = mallinfo();
          sysresume_print_mallinfo("sysresume: after mallinfo", 0, &info);
        }

      sysresume_puts("sysresume: marker after sleep\n");
      local += (uint32_t)i + (uint32_t)ret;
    }

  sysresume_printf("sysresume: %s done local=%08lx\n",
                   name, (unsigned long)local);
  return 0;
}

static void sysresume_usage(FAR const char *progname)
{
  sysresume_printf("Usage: %s [-T] [-m] [-w] [-y] [-P priority] "
                   "[-B] [-X] [-l loops] [-d usec]\n", progname);
}

static int sysresume_parse_int(FAR const char *text, FAR long *value)
{
  FAR char *endptr;
  long tmp;

  errno = 0;
  tmp = strtol(text, &endptr, 0);
  if (errno != 0 || endptr == text || *endptr != '\0')
    {
      return -EINVAL;
    }

  *value = tmp;
  return 0;
}

static int sysresume_parse_args(int argc, FAR char *argv[],
                                FAR struct sysresume_options_s *options)
{
  long value;
  int option;
  int ret;

  options->loops = 1;
  options->child_priority = CONFIG_TESTING_SYSCALL_RESUME_PRIORITY;
  options->delay_usec = CONFIG_TESTING_SYSCALL_RESUME_DELAY_USEC;
  options->child_task = false;
  options->mallinfo_after_sleep = false;
  options->wait_child = false;
  options->yield_after_notify = false;
  options->sleep_after_notify = false;
  options->direct_exit_after_notify = false;
  options->child_priority_set = false;

  while ((option = getopt(argc, argv, "BTXd:l:mwhyP:")) != ERROR)
    {
      switch (option)
        {
          case 'B':
            options->sleep_after_notify = true;
            break;

          case 'T':
            options->child_task = true;
            break;

          case 'X':
            options->direct_exit_after_notify = true;
            break;

          case 'm':
            options->mallinfo_after_sleep = true;
            break;

          case 'd':
            ret = sysresume_parse_int(optarg, &value);
            if (ret < 0 || value < 0)
              {
                sysresume_printf("sysresume: invalid delay: %s\n", optarg);
                return -EINVAL;
              }

            options->delay_usec = (useconds_t)value;
            break;

          case 'l':
            ret = sysresume_parse_int(optarg, &value);
            if (ret < 0 || value <= 0)
              {
                sysresume_printf("sysresume: invalid loop count: %s\n",
                                 optarg);
                return -EINVAL;
              }

            options->loops = (int)value;
            break;

          case 'w':
            options->wait_child = true;
            break;

          case 'y':
            options->yield_after_notify = true;
            break;

          case 'P':
            ret = sysresume_parse_int(optarg, &value);
            if (ret < 0 || value < 0)
              {
                sysresume_printf("sysresume: invalid priority: %s\n",
                                 optarg);
                return -EINVAL;
              }

            options->child_priority = (int)value;
            options->child_priority_set = true;
            break;

          case 'h':
            sysresume_usage(argv[0]);
            return 1;

          default:
            sysresume_usage(argv[0]);
            return -EINVAL;
        }
    }

  return 0;
}

static int sysresume_wait_child_priority(int priority)
{
  int max_priority;

  max_priority = sched_get_priority_max(SCHED_RR);
  if (max_priority >= 0 && priority < max_priority)
    {
      return priority + 1;
    }

  return priority;
}

static int sysresume_child_main(int argc, FAR char *argv[])
{
  int ret;

  (void)argc;
  (void)argv;

  ret = sysresume_run("child", &g_child_options);

  if (g_child_notify)
    {
      int post_errno;
      int post_ret;
      int sleep_ret;
      int yield_ret;

      g_child_status = ret;
      errno = 0;
      post_ret = sem_post(&g_child_done);
      post_errno = errno;
      sysresume_printf("sysresume: child sem_post ret=%d errno=%d\n",
                       post_ret, post_errno);

      if (post_ret < 0)
        {
          sysresume_printf("sysresume: child sem_post failed errno=%d\n",
                           post_errno);
        }
      else if (g_child_options.yield_after_notify)
        {
          errno = 0;
          sysresume_puts("sysresume: child sched_yield begin\n");
          yield_ret = sched_yield();
          sysresume_printf("sysresume: child sched_yield ret=%d errno=%d\n",
                           yield_ret, errno);
        }

      if (post_ret >= 0 && g_child_options.sleep_after_notify)
        {
          errno = 0;
          sysresume_puts("sysresume: child post-notify sleep begin\n");
          sleep_ret = usleep(g_child_options.delay_usec);
          sysresume_printf("sysresume: child post-notify sleep ret=%d "
                           "errno=%d\n",
                           sleep_ret, errno);
        }
    }

  if (g_child_options.direct_exit_after_notify)
    {
      sysresume_puts("sysresume: child _exit begin\n");
      _exit(ret);
    }

  sysresume_puts("sysresume: child return\n");
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  struct sysresume_options_s options;
  int wait_status;
  int status;
  int pid;
  int ret;

  ret = sysresume_parse_args(argc, argv, &options);
  if (ret != 0)
    {
      return ret > 0 ? 0 : EXIT_FAILURE;
    }

  if (!options.child_task)
    {
      return sysresume_run("main", &options);
    }

  if (options.wait_child && !options.child_priority_set)
    {
      options.child_priority =
        sysresume_wait_child_priority(options.child_priority);
    }

  g_child_options = options;
  g_child_options.child_task = false;
  g_child_notify = false;
  g_child_status = EXIT_FAILURE;

  if (options.wait_child)
    {
      ret = sem_init(&g_child_done, 0, 0);
      if (ret < 0)
        {
          sysresume_printf("sysresume: sem_init failed errno=%d\n", errno);
          return EXIT_FAILURE;
        }

      g_child_notify = true;
    }

  pid = task_create("sysresume-child",
                    options.child_priority,
                    CONFIG_TESTING_SYSCALL_RESUME_STACKSIZE,
                    sysresume_child_main, NULL);
  if (pid < 0)
    {
      sysresume_printf("sysresume: task_create failed errno=%d\n", errno);
      if (options.wait_child)
        {
          g_child_notify = false;
          sem_destroy(&g_child_done);
        }

      return EXIT_FAILURE;
    }

  sysresume_printf("sysresume: started child pid=%d wait=%d prio=%d\n",
                   pid, options.wait_child ? 1 : 0,
                   options.child_priority);

  if (!options.wait_child)
    {
      return EXIT_SUCCESS;
    }

  status = 0;
  do
    {
      errno = 0;
      ret = sem_wait(&g_child_done);
    }
  while (ret < 0 && errno == EINTR);

  status = g_child_status;
  g_child_notify = false;
  sysresume_printf("sysresume: child wait ret=%d status=%d errno=%d\n",
                   ret, status, errno);
  sem_destroy(&g_child_done);

  errno = 0;
  wait_status = 0;
  ret = waitpid(pid, &wait_status, WNOHANG);
  sysresume_printf("sysresume: waitpid WNOHANG ret=%d status=%d errno=%d\n",
                   ret, wait_status, errno);

  return status;
}
