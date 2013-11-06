/*************************************************************************
    > File Name: threadpool.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月05日 星期二 15时38分57秒
 ************************************************************************/
#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include<pthread.h>

typedef struct tpool_work
{
	void* (*fun)(void*);
	void* arg;
	struct tpool_work* next;
}tpool_work_t;

typedef struct tpool
{
	int shutdown;
	int max_thread_num;
	pthread_t* pid;
	tpool_work_t* queue_head;
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_ready;
}tpool_t;

int tpool_init(int max_thread_num);
void tpool_uninit();
int tpool_add_work(void* (*fun)(void*),void* arg);
#endif



