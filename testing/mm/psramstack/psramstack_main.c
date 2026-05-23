/****************************************************************************
 * apps/testing/mm/psramstack/psramstack_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PSRAMSTACK_INTSRAM_BASE  0x34000000ul
#define PSRAMSTACK_INTSRAM_END   0x34200000ul
#define PSRAMSTACK_PSRAM_BASE    0x90000000ul
#define PSRAMSTACK_PSRAM_END     0x92000000ul

#define PSRAMSTACK_STACK_ALIGN   32
#define PSRAMSTACK_MAX_HELD      64
#define PSRAMSTACK_DEFAULT_LOOPS 20000
#define PSRAMSTACK_DEFAULT_STACK 8192
#define PSRAMSTACK_TOUCH_DEPTH   6
#define PSRAMSTACK_TOUCH_BYTES   192
#define NSEC_PER_SEC_U64         1000000000ull

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct psramstack_options_s
{
  unsigned int loops;
  size_t stacksize;
};

struct psramstack_alloc_s
{
  FAR void *stack;
  FAR void *held[PSRAMSTACK_MAX_HELD];
  unsigned int nheld;
};

struct psramstack_worker_s
{
  FAR const char *name;
  unsigned int loops;
  size_t touch_bytes;
  bool touch_each_loop;
  bool sys_call_each_loop;
  bool syscall_each_loop;
  bool yield_each_loop;
  sem_t started;
  FAR volatile uint32_t *sink;
  uintptr_t local_addr;
};

struct psramstack_pingpong_s
{
  FAR const char *name;
  unsigned int loops;
  sem_t main_sem;
  sem_t worker_sem;
  FAR volatile uint32_t *sink;
  uintptr_t local_addr;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile uint32_t g_psramstack_sink;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool psramstack_range_contains(uintptr_t addr, size_t size,
                                      uintptr_t base, uintptr_t end)
{
  return addr >= base && addr < end && size <= end - addr;
}

static FAR const char *psramstack_region_name(FAR const void *ptr)
{
  uintptr_t addr = (uintptr_t)ptr;

  if (addr >= PSRAMSTACK_PSRAM_BASE && addr < PSRAMSTACK_PSRAM_END)
    {
      return "psram";
    }

  if (addr >= PSRAMSTACK_INTSRAM_BASE && addr < PSRAMSTACK_INTSRAM_END)
    {
      return "internal";
    }

  return "other";
}

static uint64_t psramstack_now_nsec(void)
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
    {
      return 0;
    }

  return (uint64_t)ts.tv_sec * NSEC_PER_SEC_U64 + (uint64_t)ts.tv_nsec;
}

static unsigned long psramstack_avg_nsec(uint64_t elapsed,
                                         unsigned int operations)
{
  if (operations == 0)
    {
      return 0;
    }

  return (unsigned long)(elapsed / operations);
}

static void psramstack_free_alloc(FAR struct psramstack_alloc_s *alloc)
{
  unsigned int i;

  if (alloc->stack != NULL)
    {
      free(alloc->stack);
      alloc->stack = NULL;
    }

  for (i = 0; i < alloc->nheld; i++)
    {
      free(alloc->held[i]);
      alloc->held[i] = NULL;
    }

  alloc->nheld = 0;
}

static int psramstack_alloc_stack(FAR struct psramstack_alloc_s *alloc,
                                  size_t stacksize, uintptr_t base,
                                  uintptr_t end)
{
  unsigned int i;

  memset(alloc, 0, sizeof(*alloc));

  for (i = 0; i < PSRAMSTACK_MAX_HELD; i++)
    {
      FAR void *ptr = memalign(PSRAMSTACK_STACK_ALIGN, stacksize);
      uintptr_t addr = (uintptr_t)ptr;

      if (ptr == NULL)
        {
          return -ENOMEM;
        }

      if (psramstack_range_contains(addr, stacksize, base, end))
        {
          alloc->stack = ptr;
          return OK;
        }

      alloc->held[alloc->nheld++] = ptr;
    }

  return -ENOENT;
}

static void psramstack_touch_stack(FAR volatile uint32_t *sink,
                                   unsigned int depth,
                                   size_t bytes)
{
  volatile uint8_t local[PSRAMSTACK_TOUCH_BYTES];
  size_t limit = bytes;
  size_t i;

  if (limit > sizeof(local))
    {
      limit = sizeof(local);
    }

  for (i = 0; i < limit; i++)
    {
      local[i] = (uint8_t)(i + depth);
    }

  *sink += local[(limit == 0) ? 0 : limit - 1];

  if (depth > 0)
    {
      psramstack_touch_stack(sink, depth - 1, bytes);
    }
}

static FAR void *psramstack_worker(FAR void *arg)
{
  FAR struct psramstack_worker_s *worker = arg;
  volatile uint32_t marker = 0;
  unsigned int i;

  worker->local_addr = (uintptr_t)&marker;
  sem_post(&worker->started);

  for (i = 0; i < worker->loops; i++)
    {
      if (worker->touch_each_loop)
        {
          psramstack_touch_stack(worker->sink, PSRAMSTACK_TOUCH_DEPTH,
                                 worker->touch_bytes);
        }

      if (worker->sys_call_each_loop)
        {
          *worker->sink += (uint32_t)(sched_getcpu() + 1);
        }

      if (worker->syscall_each_loop)
        {
          *worker->sink += (uint32_t)time(NULL);
        }

      if (worker->yield_each_loop)
        {
          sched_yield();
        }
    }

  *worker->sink += marker;
  return NULL;
}

static FAR void *psramstack_pingpong_worker(FAR void *arg)
{
  FAR struct psramstack_pingpong_s *pong = arg;
  volatile uint32_t marker = 0;
  unsigned int i;

  pong->local_addr = (uintptr_t)&marker;

  for (i = 0; i < pong->loops; i++)
    {
      sem_wait(&pong->worker_sem);
      marker += i;
      sem_post(&pong->main_sem);
    }

  *pong->sink += marker;
  return NULL;
}

static int psramstack_run_thread(FAR const char *name, FAR void *stack,
                                 size_t stacksize, unsigned int loops,
                                 bool touch_each_loop,
                                 bool sys_call_each_loop,
                                 bool syscall_each_loop,
                                 bool yield_each_loop,
                                 FAR uint64_t *elapsed)
{
  struct psramstack_worker_s worker;
  pthread_attr_t attr;
  pthread_t tid;
  uint64_t start;
  int ret;

  memset(&worker, 0, sizeof(worker));
  worker.name        = name;
  worker.loops       = loops;
  worker.touch_bytes = PSRAMSTACK_TOUCH_BYTES;
  worker.touch_each_loop = touch_each_loop;
  worker.sys_call_each_loop = sys_call_each_loop;
  worker.syscall_each_loop = syscall_each_loop;
  worker.yield_each_loop = yield_each_loop;
  worker.sink        = &g_psramstack_sink;
  sem_init(&worker.started, 0, 0);

  pthread_attr_init(&attr);
  ret = pthread_attr_setstack(&attr, stack, stacksize);
  if (ret != OK)
    {
      printf("psramstack: %s pthread_attr_setstack failed: %d\n",
             name, ret);
      sem_destroy(&worker.started);
      return -ret;
    }

  start = psramstack_now_nsec();
  ret = pthread_create(&tid, &attr, psramstack_worker, &worker);
  if (ret != OK)
    {
      printf("psramstack: %s pthread_create failed: %d\n", name, ret);
      sem_destroy(&worker.started);
      return -ret;
    }

  sem_wait(&worker.started);
  printf("psramstack: %s stack=%p size=%zu local=%p local-region=%s\n",
         name, stack, stacksize, (FAR void *)worker.local_addr,
         psramstack_region_name((FAR void *)worker.local_addr));

  pthread_join(tid, NULL);
  *elapsed = psramstack_now_nsec() - start;
  sem_destroy(&worker.started);
  pthread_attr_destroy(&attr);
  return OK;
}

static int psramstack_run_pingpong(FAR const char *name, FAR void *stack,
                                   size_t stacksize, unsigned int loops,
                                   FAR uint64_t *elapsed)
{
  struct psramstack_pingpong_s pong;
  pthread_attr_t attr;
  pthread_t tid;
  uint64_t start;
  unsigned int i;
  int ret;

  memset(&pong, 0, sizeof(pong));
  pong.name  = name;
  pong.loops = loops;
  pong.sink  = &g_psramstack_sink;
  sem_init(&pong.main_sem, 0, 0);
  sem_init(&pong.worker_sem, 0, 0);

  pthread_attr_init(&attr);
  ret = pthread_attr_setstack(&attr, stack, stacksize);
  if (ret != OK)
    {
      printf("psramstack: %s pingpong attr failed: %d\n", name, ret);
      sem_destroy(&pong.main_sem);
      sem_destroy(&pong.worker_sem);
      return -ret;
    }

  ret = pthread_create(&tid, &attr, psramstack_pingpong_worker, &pong);
  if (ret != OK)
    {
      printf("psramstack: %s pingpong create failed: %d\n", name, ret);
      sem_destroy(&pong.main_sem);
      sem_destroy(&pong.worker_sem);
      return -ret;
    }

  start = psramstack_now_nsec();
  for (i = 0; i < loops; i++)
    {
      sem_post(&pong.worker_sem);
      sem_wait(&pong.main_sem);
    }

  pthread_join(tid, NULL);
  *elapsed = psramstack_now_nsec() - start;

  printf("psramstack: %s pingpong-local=%p local-region=%s\n",
         name, (FAR void *)pong.local_addr,
         psramstack_region_name((FAR void *)pong.local_addr));

  sem_destroy(&pong.main_sem);
  sem_destroy(&pong.worker_sem);
  pthread_attr_destroy(&attr);
  return OK;
}

static void psramstack_usage(FAR const char *progname)
{
  printf("Usage: %s [-l loops] [-s stack-size]\n", progname);
  printf("  -l loops       Loop count per stack test, default %u\n",
         PSRAMSTACK_DEFAULT_LOOPS);
  printf("  -s stack-size  Explicit pthread stack bytes, default %u\n",
         PSRAMSTACK_DEFAULT_STACK);
}

static int psramstack_parse_int(FAR const char *text, FAR long *value)
{
  FAR char *endptr;
  long parsed;

  errno = 0;
  parsed = strtol(text, &endptr, 0);
  if (errno != 0 || endptr == text || *endptr != '\0')
    {
      return -EINVAL;
    }

  *value = parsed;
  return OK;
}

static int psramstack_parse_args(int argc, FAR char *argv[],
                                 FAR struct psramstack_options_s *options)
{
  int opt;

  options->loops = PSRAMSTACK_DEFAULT_LOOPS;
  options->stacksize = PSRAMSTACK_DEFAULT_STACK;

  while ((opt = getopt(argc, argv, "hl:s:")) != ERROR)
    {
      long value;
      int ret;

      switch (opt)
        {
          case 'l':
            ret = psramstack_parse_int(optarg, &value);
            if (ret < 0 || value <= 0)
              {
                printf("psramstack: invalid loop count: %s\n", optarg);
                return -EINVAL;
              }

            options->loops = (unsigned int)value;
            break;

          case 's':
            ret = psramstack_parse_int(optarg, &value);
            if (ret < 0 || value < PTHREAD_STACK_MIN)
              {
                printf("psramstack: invalid stack size: %s\n", optarg);
                return -EINVAL;
              }

            options->stacksize = (size_t)value;
            break;

          case 'h':
            psramstack_usage(argv[0]);
            return 1;

          default:
            psramstack_usage(argv[0]);
            return -EINVAL;
        }
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  struct psramstack_options_s options;
  struct psramstack_alloc_s internal;
  struct psramstack_alloc_s psram;
  uint64_t internal_yield_elapsed = 0;
  uint64_t psram_yield_elapsed = 0;
  uint64_t internal_syscallonly_elapsed = 0;
  uint64_t psram_syscallonly_elapsed = 0;
  uint64_t internal_syscall_elapsed = 0;
  uint64_t psram_syscall_elapsed = 0;
  uint64_t internal_yieldonly_elapsed = 0;
  uint64_t psram_yieldonly_elapsed = 0;
  uint64_t internal_touch_elapsed = 0;
  uint64_t psram_touch_elapsed = 0;
  uint64_t internal_ping_elapsed = 0;
  uint64_t psram_ping_elapsed = 0;
  int ret;
  int exitcode = EXIT_SUCCESS;

  ret = psramstack_parse_args(argc, argv, &options);
  if (ret > 0)
    {
      return EXIT_SUCCESS;
    }

  if (ret < 0)
    {
      return EXIT_FAILURE;
    }

  printf("psramstack: loops=%u stack=%zu\n",
         options.loops, options.stacksize);

  ret = psramstack_alloc_stack(&internal, options.stacksize,
                               PSRAMSTACK_INTSRAM_BASE,
                               PSRAMSTACK_INTSRAM_END);
  if (ret < 0)
    {
      printf("psramstack: internal stack allocation failed: %d\n", ret);
      return EXIT_FAILURE;
    }

  ret = psramstack_alloc_stack(&psram, options.stacksize,
                               PSRAMSTACK_PSRAM_BASE,
                               PSRAMSTACK_PSRAM_END);
  if (ret < 0)
    {
      printf("psramstack: psram stack allocation failed: %d\n", ret);
      psramstack_free_alloc(&internal);
      return EXIT_FAILURE;
    }

  printf("psramstack: internal-stack=%p held=%u psram-stack=%p held=%u\n",
         internal.stack, internal.nheld, psram.stack, psram.nheld);

  ret = psramstack_run_thread("internal-touch", internal.stack,
                              options.stacksize, options.loops,
                              true, false, false, false,
                              &internal_touch_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("psram-touch", psram.stack,
                              options.stacksize, options.loops,
                              true, false, false, false,
                              &psram_touch_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("internal-syscallonly", internal.stack,
                              options.stacksize, options.loops,
                              false, true, false, false,
                              &internal_syscallonly_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("psram-syscallonly", psram.stack,
                              options.stacksize, options.loops,
                              false, true, false, false,
                              &psram_syscallonly_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("internal-timeonly", internal.stack,
                              options.stacksize, options.loops,
                              false, false, true, false,
                              &internal_syscall_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("psram-timeonly", psram.stack,
                              options.stacksize, options.loops,
                              false, false, true, false,
                              &psram_syscall_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("internal-yieldonly", internal.stack,
                              options.stacksize, options.loops,
                              false, false, false, true,
                              &internal_yieldonly_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("psram-yieldonly", psram.stack,
                              options.stacksize, options.loops,
                              false, false, false, true,
                              &psram_yieldonly_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("internal-yield", internal.stack,
                              options.stacksize, options.loops,
                              true, false, false, true,
                              &internal_yield_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_thread("psram-yield", psram.stack,
                              options.stacksize, options.loops,
                              true, false, false, true,
                              &psram_yield_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_pingpong("internal", internal.stack,
                                options.stacksize, options.loops,
                                &internal_ping_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  ret = psramstack_run_pingpong("psram", psram.stack,
                                options.stacksize, options.loops,
                                &psram_ping_elapsed);
  if (ret < 0)
    {
      exitcode = EXIT_FAILURE;
    }

  printf("psramstack: result internal-touch=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)internal_touch_elapsed,
         psramstack_avg_nsec(internal_touch_elapsed, options.loops));
  printf("psramstack: result psram-touch=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)psram_touch_elapsed,
         psramstack_avg_nsec(psram_touch_elapsed, options.loops));
  printf("psramstack: result internal-syscallonly=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)internal_syscallonly_elapsed,
         psramstack_avg_nsec(internal_syscallonly_elapsed, options.loops));
  printf("psramstack: result psram-syscallonly=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)psram_syscallonly_elapsed,
         psramstack_avg_nsec(psram_syscallonly_elapsed, options.loops));
  printf("psramstack: result internal-timeonly=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)internal_syscall_elapsed,
         psramstack_avg_nsec(internal_syscall_elapsed, options.loops));
  printf("psramstack: result psram-timeonly=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)psram_syscall_elapsed,
         psramstack_avg_nsec(psram_syscall_elapsed, options.loops));
  printf("psramstack: result internal-yieldonly=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)internal_yieldonly_elapsed,
         psramstack_avg_nsec(internal_yieldonly_elapsed, options.loops));
  printf("psramstack: result psram-yieldonly=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)psram_yieldonly_elapsed,
         psramstack_avg_nsec(psram_yieldonly_elapsed, options.loops));
  printf("psramstack: result internal-yield=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)internal_yield_elapsed,
         psramstack_avg_nsec(internal_yield_elapsed, options.loops));
  printf("psramstack: result psram-yield=%llu ns avg=%lu ns/loop\n",
         (unsigned long long)psram_yield_elapsed,
         psramstack_avg_nsec(psram_yield_elapsed, options.loops));
  printf("psramstack: result internal-pingpong=%llu ns avg=%lu ns/round\n",
         (unsigned long long)internal_ping_elapsed,
         psramstack_avg_nsec(internal_ping_elapsed, options.loops));
  printf("psramstack: result psram-pingpong=%llu ns avg=%lu ns/round\n",
         (unsigned long long)psram_ping_elapsed,
         psramstack_avg_nsec(psram_ping_elapsed, options.loops));
  printf("psramstack: sink=%lu\n", (unsigned long)g_psramstack_sink);

  psramstack_free_alloc(&psram);
  psramstack_free_alloc(&internal);
  return exitcode;
}
