#include "thread.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct thread_t
{
	pthread_t thread;

	int idle, quit, stop;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	thread_task_t task;
	void *task_arg;
};

void *thread_loop(void *arg)
{
	thread_t *th = arg;

	for (;;)
	{
		pthread_mutex_lock(&th->mutex);
		th->idle = 1;

		LOG_FMT(LVL_DEBUG, "(%p) - now idle", (void *)th);
		pthread_cond_signal(&th->cond);

		while (th->idle)
			pthread_cond_wait(&th->cond, &th->mutex);
		
		if (th->quit)
		{
			pthread_mutex_unlock(&th->mutex);
			break;
		}

		pthread_mutex_unlock(&th->mutex);

		LOG_FMT(LVL_DEBUG, "(%p) - starting task", (void *)th);
		th->task(th, th->task_arg);
	}

	return NULL;
}

thread_t *thread_init(thread_task_t task, void *arg)
{
	thread_t *th = safe_malloc(sizeof(thread_t));
	th->task = task;
	th->task_arg = arg;

	th->idle = 0;
	th->quit = 0;
	th->stop = 0;

	pthread_mutex_init(&th->mutex, NULL);
	pthread_cond_init(&th->cond, NULL);
	pthread_create(&th->thread, NULL, thread_loop, th);

	LOG_FMT(LVL_DEBUG, "(%p) - created new thread", (void *)th);

	thread_wait_until_idle(th);

	return th;
}

void thread_destroy(thread_t *th)
{
	assert(thread_is_idle(th));

	LOG_FMT(LVL_DEBUG, "(%p) - destroying thread", (void *)th);

	pthread_mutex_lock(&th->mutex);
	th->idle = 0;
	th->quit = 1;
	pthread_cond_signal(&th->cond);
	pthread_mutex_unlock(&th->mutex);
	pthread_join(th->thread, NULL);

	pthread_mutex_destroy(&th->mutex);
	pthread_cond_destroy(&th->cond);
	
	free(th);
}

void thread_set_task(thread_t *th, thread_task_t task, void *arg)
{
	assert(thread_is_idle(th));

	pthread_mutex_lock(&th->mutex);
	th->task = task;
	th->task_arg = arg;
	pthread_mutex_unlock(&th->mutex);
}

void thread_set_task_arg(thread_t *th, void *arg)
{
	assert(thread_is_idle(th));

	pthread_mutex_lock(&th->mutex);
	th->task_arg = arg;
	pthread_mutex_unlock(&th->mutex);
}

void *thread_get_task_arg(thread_t *th)
{
	return th->task_arg;
}

int thread_is_idle(thread_t *th)
{
	pthread_mutex_lock(&th->mutex);
	int idle = th->idle;
	pthread_mutex_unlock(&th->mutex);

	return idle;
}

int thread_should_stop(thread_t *th)
{
	pthread_mutex_lock(&th->mutex);
	int stop = th->stop;
	pthread_mutex_unlock(&th->mutex);

	return stop;
}

void thread_wait_until_idle(thread_t *th)
{
	LOG_FMT(LVL_DEBUG, "(%p) - waiting until idle", (void *)th);

	pthread_mutex_lock(&th->mutex);
	while (!th->idle)
			pthread_cond_wait(&th->cond, &th->mutex);
	
	pthread_mutex_unlock(&th->mutex);
}

void thread_start(thread_t *th)
{
	assert(thread_is_idle(th));

	LOG_FMT(LVL_DEBUG, "(%p) - waking thread", (void *)th);

	pthread_mutex_lock(&th->mutex);
	th->idle = 0;
	th->stop = 0;
	pthread_cond_signal(&th->cond);
	pthread_mutex_unlock(&th->mutex);
}

void thread_stop(thread_t *th)
{
	LOG_FMT(LVL_DEBUG, "(%p) - stopping thread", (void *)th);

	pthread_mutex_lock(&th->mutex);
	th->stop = 1;
	pthread_mutex_unlock(&th->mutex);
}
