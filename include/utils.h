#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ONE_SEC_IN_NS 1000000000LL

// TODO: Use SDL log
#define PANIC(fmt, ...)                                                        \
  do {                                                                         \
    fprintf(stderr, "[PANIC] %s:%d in %s(): " fmt "\n", __FILE__, __LINE__,    \
            __func__, ##__VA_ARGS__);                                          \
    abort();                                                                   \
  } while (0)

#define ASSERT_NOT_NULL(ptr)                                                   \
  do {                                                                         \
    if ((ptr) == NULL) {                                                       \
      PANIC("Null pointer assertion failed: " #ptr);                           \
    }                                                                          \
  } while (0)

static inline void add_ns_to_timespec(struct timespec *ts,
                                      long long ns_to_add) {
  ts->tv_nsec += ns_to_add;
  while (ts->tv_nsec >= ONE_SEC_IN_NS) {
    ts->tv_nsec -= ONE_SEC_IN_NS;
    ts->tv_sec++;
  }
}

#endif // UTILS_H