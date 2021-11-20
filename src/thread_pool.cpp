#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "thread_pool.h"

struct threadpool *threadpool_init(int thread_num, int queue_max_num)
{
    struct threadpool *pool = NULL;

    do {
        pool = (struct threadpool *)calloc(1, sizeof(struct threadpool));
        if (!pool) {
            printf("calloc error: %m\n");
            break;
        }
        pool->thread_num = thread_num;
        pool->queue_max_num = queue_max_num;
        pool->queue_cur_num = 0;
        pool->head = NULL;
        pool->tail = NULL;
        if (pthread_mutex_init(&(pool->mutex), NULL)) {
            printf("init mutex error: %m\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_empty), NULL)) {
            printf("init queue_empty error: %m\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_empty), NULL)) {
            printf("init queue_not_empty error: %m\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_full), NULL)) {
            printf("init queue_not_full error: %m\n");
            break;
        }
        pool->pthreads = (pthread_t *)calloc(1, sizeof(pthread_t) * thread_num);
        if (!pool->pthreads) {
            printf("calloc pthreads error: %m\n");
            break;
        }
        pool->queue_close = 0;
        pool->pool_close = 0;
        int i;
        for (i = 0; i < pool->thread_num; i++) {
            pthread_create(&(pool->pthreads[i]), NULL, threadpool_function, (void *)pool);
        }
        return pool;
    } while (0);

    return NULL;
}

int threadpool_add_job(struct threadpool *pool, void *(*callback_function)(void *arg), void *arg)
{
    assert(pool != NULL);
    assert(callback_function != NULL);
    assert(arg != NULL);

    pthread_mutex_lock(&(pool->mutex));
    //如果作业等待队列满，并且线程池未关闭，等待
    while ((pool->queue_cur_num == pool->queue_max_num) && !(pool->queue_close || pool->pool_close)) {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->mutex));
    }
    //线程池关闭，终止
    if (pool->queue_close || pool->pool_close) {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    struct job *pjob = (struct job*) calloc(1, sizeof(struct job));
    if (!pjob) {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }

    pjob->callback_function = callback_function;
    pjob->arg = arg;
    pjob->next = NULL;
    if (pool->head == NULL) {
        pool->head = pool->tail = pjob;
        pthread_cond_broadcast(&(pool->queue_not_empty));
    } else {
        pool->tail->next = pjob;
        pool->tail = pjob;
    }

    pool->queue_cur_num++;
    pthread_mutex_unlock(&(pool->mutex));

    return 0;
}

void *threadpool_function(void *arg)
{
    struct threadpool *pool = (struct threadpool *)arg;
    struct job *pjob = NULL;

    while (1) {
        pthread_mutex_lock(&(pool->mutex));
        //如果作业队列为空，并且线程池未终止，等待
        //意味着已加入队列的作业可以继续执行，未加入的不能加入
        while ((pool->queue_cur_num == 0) && !pool->pool_close) {
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->mutex));
        }

        if (pool->pool_close) {
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(NULL);
        }
        pool->queue_cur_num--;
        pjob = pool->head;
        if (pool->queue_cur_num == 0) {
            pool->head = pool->tail = NULL;
        } else {
            pool->head = pjob->next;
        }

        if (pool->queue_cur_num == 0) {
            pthread_cond_signal(&(pool->queue_empty));
        }
        if (pool->queue_cur_num == pool->queue_max_num - 1) {
            pthread_cond_broadcast(&(pool->queue_not_full));
        }
        pthread_mutex_unlock(&(pool->mutex));

        //执行线程函数
        (*(pjob->callback_function))(pjob->arg);
        free(pjob);
        pjob = NULL;
    }
}

int threadpool_destroy(struct threadpool *pool)
{
    assert(pool != NULL);
    pthread_mutex_lock(&(pool->mutex));
    if (pool->queue_close || pool->pool_close) {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    //关闭队列
    pool->queue_close = 1;
    while (pool->queue_cur_num != 0) {
        pthread_cond_wait(&(pool->queue_empty), &(pool->mutex));
    }
    //关闭线程池
    pool->pool_close = 1;
    pthread_mutex_unlock(&(pool->mutex));
    //这两个广播使等待信号量的线程得到，然后终止
    pthread_cond_broadcast(&(pool->queue_not_empty));
    pthread_cond_broadcast(&(pool->queue_not_full));

    int i;
    for (i = 0; i < pool->thread_num; i++) {
        pthread_join(pool->pthreads[i], NULL);
    }

    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->queue_empty));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));
    free(pool->pthreads);

    struct job *p;
    while (pool->head != NULL) {
        p = pool->head;
        pool->head = p->next;
        free(p);
    }
    free(pool);

    return 0;
}
