#include <pthread.h>

/*作业队列*/
struct job {
    void * (*callback_function)(void *arg); //作业函数
    void *arg;                              //参数
    struct job *next;                       //队列下一个作业
};


struct threadpool {
    int thread_num;                 //线程池线程数目
    int queue_max_num;              //等待队列最大数目
    struct job *head;               //等待队列头指针
    struct job *tail;               //等待队列尾指针
    pthread_t *pthreads;            //线程指针
    pthread_mutex_t mutex;          //互斥访问信号量
    pthread_cond_t queue_empty;     //队列为空信号量
    pthread_cond_t queue_not_empty; //队列非空信号量
    pthread_cond_t queue_not_full;  //队列非满信号量
    int queue_cur_num;              //队列中作业数目
    int queue_close;                //队列是否关闭
    int pool_close;                 //线程池是否关闭
};

struct threadpool *threadpool_init(int thread_num, int queue_max_num);

int threadpool_add_job(struct threadpool *pool, void *(*callback_function)(void *arg), void *arg);

int threadpool_destroy(struct threadpool *pool);

void *threadpool_function(void *arg);
