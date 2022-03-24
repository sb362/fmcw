#ifndef THREAD_H
#define THREAD_H

// Fix for missing type _int64 under CVI 2019 (clang 3.3)
#ifdef _CVI_
#define __MINGW32__
#endif
#include <pthread.h>
#ifdef _CVI_
#undef __MINGW32__
#endif

typedef struct thread_t thread_t;
typedef void (*thread_task_t)(thread_t *, void *);

thread_t *thread_init(thread_task_t task, void *arg);
void  thread_destroy(thread_t *th);
void  thread_set_task(thread_t *th, thread_task_t task, void *arg);
void  thread_set_task_arg(thread_t *th, void *arg);
void *thread_get_task_arg(thread_t *th);
int   thread_is_idle(thread_t *th);
int   thread_should_stop(thread_t *th);
void  thread_wait_until_idle(thread_t *th);
void  thread_start(thread_t *th);
void  thread_stop(thread_t *th);

#endif
