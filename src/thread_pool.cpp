#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "thread_pool.h"

ThreadPool::ThreadPool(int thread_num_ = 10, 
                       int queue_max_num_ = 100)
{
    do {
        thread_num = thread_num_;
        queue_max_num = queue_max_num_;
        queue_cur_num = 0;
        head = NULL;
        tail = NULL;

        if(pthread_mutex_init(&mutex, NULL)) {
            printf("Init mutex error: %m\n");
            break;
        }
        if(pthread_cond_init(&queue_empty, NULL)) {
            printf("Init queue_empty error: %m\n");
            break;
        }
        if(pthread_cond_init(&queue_not_empty, NULL)) {
            printf("Init queue_not_empty error: %m\n");
            break;
        }
        if(pthread_cond_init(&queue_not_full, NULL)) {
            printf("Init queue_not_full error: %m\n");
            break;
        }
        pthreads = new pthread_t[thread_num];
        if(pthreads == NULL) {
            printf("new pthreads error: %m\n");
            break;
        }
        queue_close = 0;
        pool_close = 0;
        for(int i = 0; i < thread_num; i++) {
            pthread_create(&(pthreads[i]), NULL,
                PthreadFunction, (void *)this);
        }
    } while(0);
}

ThreadPool::~ThreadPool()
{
    pthread_mutex_lock(&mutex);
    //如果已经在终止了，返回
    if(queue_close || pool_close){
        pthread_mutex_unlock(&mutex);
        return;
    }
    //关闭队列
    queue_close = 1;
    while(queue_cur_num > 0){
        pthread_cond_wait(&queue_empty, &mutex);
    }
    //关闭线程池
    pool_close = 1;
    pthread_mutex_unlock(&mutex);
    //广播，使等待信号的线程得到并执行，然后终止
    pthread_cond_broadcast(&queue_not_empty);
    pthread_cond_broadcast(&queue_not_full);
    //等待所有线程结束
    for(int i = 0; i < thread_num; i++){
        pthread_join(pthreads[i], NULL);
    }
    //销毁信号量和条件
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&queue_empty);
    pthread_cond_destroy(&queue_not_empty);
    pthread_cond_destroy(&queue_not_full);
    delete [] pthreads;

    Job *p = NULL;
    while(head != NULL){
        p = head;
        head = head->next;
        delete p;
    }
}

int ThreadPool::ThreadPoolAddJob(void *(*callback_function)(void *arg), void *arg)
{
    assert(callback_function != NULL);
    assert(arg != NULL);

    pthread_mutex_lock(&mutex);
    if(queue_cur_num == queue_max_num)printf("queue full\n");
    //如果作业等待队列满，并且线程池未关闭，等待
    while(queue_cur_num == queue_max_num && 
        !(queue_close || pool_close)) {
        pthread_cond_wait(&queue_not_full, &mutex);
    }
    //如果线程池或队列关闭，立即终止
    if(queue_close || pool_close){
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    Job *pjob = new Job;
    if(!pjob) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    pjob->callback_function = callback_function;
    pjob->arg = arg;
    pjob->next = NULL;
    if(head == NULL) {
        head = tail = pjob;
        pthread_cond_broadcast(&queue_not_empty);
    }else{
        tail->next = pjob;
        tail = pjob;
    }

    queue_cur_num++;
    pthread_mutex_unlock(&mutex);
    return 0;
}

void * ThreadPool::ThreadPoolFunction()
{
    Job *pjob = NULL;
    while(1) {
        pthread_mutex_lock(&mutex);
        //如果作业队列为空，并且线程池未终止，等待
        //没有判断队列是否终止，意味着即使队列终止，已加入队列的任务仍然执行
        while(queue_cur_num == 0 && !pool_close){
            pthread_cond_wait(&queue_not_empty, &mutex);
        }
        if(pool_close){
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }
        queue_cur_num--;
        pjob = head;
        if(queue_cur_num == 0) {
            head = tail = NULL;
        } else {
            head = head->next;
        }

        if(queue_cur_num == 0) {
            pthread_cond_signal(&queue_empty);
        }
        if(queue_cur_num == queue_max_num - 1) {
            pthread_cond_broadcast(&queue_not_full);
        }
        pthread_mutex_unlock(&mutex);

        //执行线程函数
        (*(pjob->callback_function))(pjob->arg);
        delete pjob;
        pjob=NULL;
    }
}


void * PthreadFunction(void *arg)
{
    ThreadPool *pool=(ThreadPool *)arg;
    pool->ThreadPoolFunction();
    return NULL;
}