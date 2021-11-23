#include <pthread.h>

/*作业队列*/
struct Job {
    void * (*callback_function)(void *arg); //作业函数
    void *arg;                              //参数
    struct Job *next;                       //队列下一个作业
};


class ThreadPool {
private:
    int thread_num;                 //线程池线程数目
    int queue_max_num;              //等待队列最大数目
    struct Job *head;               //等待队列头指针
    struct Job *tail;               //等待队列尾指针
    pthread_t *pthreads;            //线程指针
    pthread_mutex_t mutex;          //互斥访问信号量
    pthread_cond_t queue_empty;     //队列为空信号量
    pthread_cond_t queue_not_empty; //队列非空信号量
    pthread_cond_t queue_not_full;  //队列非满信号量
    int queue_cur_num;              //队列中作业数目
    int queue_close;                //队列是否关闭
    int pool_close;                 //线程池是否关闭

public:
    ThreadPool(int thread_num_, int queue_max_num_);//构造函数，创建线程池
    int ThreadPoolAddJob(void *(*callback_function)(void *arg), void *arg);//为线程池添加一个任务
    void * ThreadPoolFunction();//每个线程要执行的函数
    ~ThreadPool();//销毁线程池
};

/**
*   参数为ThreadPool *，指向线程池
*   调用ThreadPool::ThreadPoolFunction()函数，
*   返回NULL
**/
void * PthreadFunction(void *arg);