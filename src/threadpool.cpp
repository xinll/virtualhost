/*************************************************************************
    > File Name: threadpool.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月05日 星期二 15时39分11秒
 ************************************************************************/

#include "threadpool.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static tpool_t *pool = NULL;
static void* thread_routine(void *arg);

int tpool_init(int max_thread_num)
{
	int i;
	pool = (tpool_t*)malloc(sizeof(tpool_t));
	memset(pool,0,sizeof(tpool_t));
	if(!pool)
	{
		exit(1);
	}

	pool->max_thread_num = max_thread_num;
	pool->shutdown = 0;
	pool->queue_head = NULL;
	if(pthread_mutex_init(&pool->queue_lock,NULL) != 0)
	{
		free(pool);
		exit(1);
	}
	if(pthread_cond_init(&pool->queue_ready,NULL) != 0)
	{
		pthread_mutex_destroy(&pool->queue_lock);
		free(pool);
		exit(1);
	}

	pool->pid = (pthread_t*)malloc(sizeof(pthread_t) * max_thread_num);
	memset(pool->pid,0,sizeof(pthread_t) * max_thread_num);
	if(pool->pid == NULL)
	{
		pthread_mutex_destroy(&pool->queue_lock);
		pthread_cond_destroy(&pool->queue_ready);
		free(pool);
		exit(1);
	}
	for(i = 0; i < max_thread_num; i++)
	{
		if(pthread_create(&pool->pid[i],NULL,thread_routine,NULL) != 0)
		{
			pthread_mutex_destroy(&pool->queue_lock);
			pthread_cond_destroy(&pool->queue_ready);
			free(pool->pid);
			free(pool);
			exit(1);
		}
	}
	return 0;
}

static void* thread_routine(void *arg)
{
	tpool_work_t *work;
	
	while(1)
	{
		pthread_mutex_lock(&pool->queue_lock);
		while(!pool->queue_head && !pool->shutdown)
		{
			pthread_cond_wait(&pool->queue_ready,&pool->queue_lock);
		}

		if(pool->shutdown)
		{
			pthread_mutex_unlock(&pool->queue_lock);
			pthread_exit(NULL);
		}

		work = pool->queue_head;
		pool->queue_head = pool->queue_head->next;
		pthread_mutex_unlock(&pool->queue_lock);

		work->fun(work->arg);
		free(work);
	}
	return NULL;
}

void tpool_uninit()
{
	int i;
	tpool_work_t *member;
	
	if(pool->shutdown)
		return;

	pool->shutdown = 1;

	pthread_mutex_lock(&pool->queue_lock);
	pthread_cond_broadcast(&pool->queue_ready);
	pthread_mutex_unlock(&pool->queue_lock);

	for(i = 0; i < pool->max_thread_num; i++)
	{
		pthread_join(pool->pid[i],NULL);
	}

	free(pool->pid);

	while(pool->queue_head)
	{
		member = pool->queue_head;
		pool->queue_head = pool->queue_head->next;
		free(member);
	}

	pthread_mutex_destroy(&pool->queue_lock);
	pthread_cond_destroy(&pool->queue_ready);
	free(pool);
}

int tpool_add_work(void* (*fun)(void*),void* arg)
{
	tpool_work_t *work,*member;
	if(!fun)
	{
		return -1;
	}

	work = (tpool_work_t*)malloc(sizeof(tpool_work_t));
	if(!work)
	{
		return -1;
	}

	work->fun = fun;
	work->arg = arg;
	work->next = NULL;

	pthread_mutex_lock(&pool->queue_lock);
	member = pool->queue_head;
	if(!member)
		pool->queue_head = work;
	else
	{
		while(member->next)
		{
			member = member->next;
		}
		member->next = work;
	}
	pthread_cond_signal(&pool->queue_ready);
	pthread_mutex_unlock(&pool->queue_lock);
	return 0;
}
