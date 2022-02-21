#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZEOF_ARRAY(array) ((sizeof (array)) / (sizeof (array)[0]))

#ifndef LOG_LEVEL
#ifdef NDEBUG
#define LOG_LEVEL FATAL
#else
#define LOG_LEVEL DEBUG
#endif
#endif

enum LogLevel
{
  FATAL,
  WARN,
  INFO,
  DEBUG,
  TRACE
};

extern const char *log_level_strs[];

#define LOG(lvl, msg)                                            \
  do {                                                           \
    if (lvl <= LOG_LEVEL)                                        \
      fprintf(stdout, "%s %s:%d " msg "\n", log_level_strs[lvl], \
              __FUNCTION__, __LINE__);                           \
      fflush(stdout);                                            \
  } while (0)

#define LOG_FMT(lvl, fmt, ...)                                   \
  do {                                                           \
    if (lvl <= LOG_LEVEL)                                        \
      fprintf(stdout, "%s %s:%d " fmt "\n", log_level_strs[lvl], \
              __FUNCTION__, __LINE__, __VA_ARGS__);              \
      fflush(stdout);                                            \
  } while (0)

#define FATAL_ERROR(title, fmt, ...)                                \
  do {                                                              \
    fprintf(stderr, "%s %s:%d " fmt "\n", log_level_strs[FATAL],    \
            __FUNCTION__, __LINE__, __VA_ARGS__);                   \
    fflush(stderr);                                                 \
  } while (0)

void timer_init(void);
double elapsed_milliseconds(void);

void *safe_malloc(size_t size);
void *aligned_malloc(size_t size);
void aligned_free(void *mem);


#endif // UTIL_H
