/**
* File Name: ThreadPool.cpp
* Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
*
* Author: Xianwen Wang
* Version: v1.0
* Date: 2021/6/8 21:11
* LastEditTime: 2021/6/8 21:11
* Copyright: Xianwen Wang
* Project Name: CPP_ThreadPool
* IDE: CLion
* Reference:
* History: 修改历史记录列表， 每条修改记录应包括修改日期、修改者及修改内容简述
*/
//

#include "ThreadPool.h"
#include<string.h>
#include<string>
#include<iostream>
#include <unistd.h>

using namespace std;

ThreadPool::ThreadPool(int minNumThreads_, int maxNumThreads_)
{

    // 实例化任务队列
    taskQ = new TaskQueue;

    if (taskQ == nullptr)
    {
        cout << "malloc TaskQueue failure \n";

        delete taskQ;
        taskQ = nullptr;
    }

    do
    {
        threadIDs = new pthread_t[maxNumThreads_]; //


        if (threadIDs == nullptr)  // 内存分配失败
        {
            cout << "malloc threadpool failure \n";
            break;
        }


        memset(threadIDs, 0, sizeof(threadIDs) * maxNumThreads_); // 初始化


        this->minNumThreads = minNumThreads_;
        this->maxNumThreads = maxNumThreads_;
        this->busyNumThreads = 0;
        this->liveNumThreads = minNumThreads_; // 与最小个数相等
        this->destroyNumThreads = 0;

        // 初始化互斥锁与条件量
        if (pthread_mutex_init(&mutexPool, NULL) != 0 || pthread_mutex_init(&mutexBusy, NULL) != 0 ||
            pthread_cond_init(&notEmpty, NULL) != 0)
        {
            cout << "mutex or pthread_cond  initialization failure \n";
            break;
        }


        this->shutdownThreadPool = false; // 默认不销毁


        // 创建线程
        int ret = pthread_create(&this->threadManagerID, NULL, manager, this);    // 创建管理者线程

        cout << "create manmager thread , ret= " << ret << endl;
        for (int i = 0; i < this->minNumThreads; i++)   // 创建工作线程
        {
            int ret = pthread_create(&this->threadIDs[i], NULL, worker, this);
            cout << "create work thread " << i << ",ret= " << ret << endl;

        }

        return;

    } while (0);

    // 释放资源
    if (this->threadIDs)
    {
        delete[]  threadIDs;
        threadIDs = nullptr;
    }
    if (taskQ)
    {
        delete taskQ;
        taskQ = nullptr;
    }


}

void *ThreadPool::manager(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg)  ;

    while (!pool->shutdownThreadPool)
    {

        //每隔3秒检测一次
        sleep(3);

        // 取出线程池中任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->taskQ->getTaskNumber();
        int liveNum = pool->liveNumThreads;
        int maxNumThreads=pool->maxNumThreads;
        int minNumThreads=pool->minNumThreads;
        pthread_mutex_unlock(&pool->mutexPool);

        // 取出忙的线程的数量
        pthread_mutex_lock(&pool->mutexBusy);
        int busyNum = pool->busyNumThreads;
        pthread_mutex_unlock(&pool->mutexBusy);

        // 添加线程算法
        // 任务的个数>存活的线程个数 && 存活的线程数< 最大线程数
        if (queueSize > liveNum && liveNum < maxNumThreads)
        {
            pthread_mutex_lock(&pool->mutexPool);
            int counter = 0;
            for (int i = 0; i < maxNumThreads && counter < NUMBER && liveNum < maxNumThreads; i++)
            {
                if (pool->threadIDs[i] == 0)  // 找到空位 即 数组中空闲的位置
                {
                    pthread_create(& pool->threadIDs[i], NULL, worker, pool);
                    counter++;
                    pool->liveNumThreads++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);


        }

        // 销毁线程算法
        // 忙的线程*2 < 存活的线程个数 && 存活的线程 > 最小线程数
        if (busyNum * 2 < liveNum && liveNum > minNumThreads)
        {
            pthread_mutex_lock(&pool->mutexPool);

            pool->destroyNumThreads = NUMBER;
            pthread_mutex_unlock(&pool->mutexPool);

            // 让工作的线程自杀
            for (int i = 0; i < NUMBER; i++)
            {
                pthread_cond_signal(&pool->notEmpty);  // 唤醒阻塞在条件变量的线程
            }


        }


    }


    return nullptr;
}

[[noreturn]] void *ThreadPool::worker(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);

    while (true)
    {
        pthread_mutex_lock(&pool->mutexPool);    // 加锁

        // 当前任务队列是否为空
        while (pool->taskQ->getTaskNumber() == 0 && !pool->shutdownThreadPool)
        {
            // 阻塞工作线程ID
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

            // 线程阻塞函数, 哪个线程调用这个函数, 哪个线程就会被阻塞
//            int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
//            通过函数原型可以看出，该函数在阻塞线程的时候，需要一个互斥锁参数，这个互斥锁主要功能是进行线程同步，让线程顺序进入临界区，避免出现数共享资源的数据混乱。该函数会对这个互斥锁做以下几件事情：
//
//            在阻塞线程时候，如果线程已经对互斥锁 mutex 上锁，那么会将这把锁打开，这样做是为了避免死锁
//            当线程解除阻塞的时候，函数内部会帮助这个线程再次将这个 mutex 互斥锁锁上，继续向下访问临界区


            // 判断是不是要销毁线程
            if (pool->destroyNumThreads > 0)
            {

                pool->destroyNumThreads--;

                if (pool->liveNumThreads > pool->minNumThreads)
                {
                    pool->liveNumThreads--;
                    pthread_mutex_unlock(&pool->mutexPool);

                    //   pthread_exit(NULL);
                    pool->threadExit();  // 销毁当前线程
                }


            }

        }

        // 判断线程池是否被关闭了
        if (pool->shutdownThreadPool)
        {
            pthread_mutex_unlock(&pool->mutexPool);

            //pthread_exit(NULL);
            pool->threadExit();
        }

        // 从任务队列中取出一个任务
        Task task = pool->taskQ->taskTask();



        //    pthread_cond_signal(&pool->notFull);   // 唤醒生产者  生产者不会阻塞


        cout << "thread " << to_string(pthread_self()) << " start working --- \n";

        pthread_mutex_unlock(&pool->mutexPool);  // 解锁


        pthread_mutex_lock(&pool->mutexBusy);   // busyNumThreads 专用的互斥锁
        pool->busyNumThreads++;
        pthread_mutex_unlock(&pool->mutexBusy);

        task.functions(task.arg);    // 工作
        delete task.arg;
        task.arg = nullptr;

        cout << "thread " << to_string(pthread_self()) << " end working --- \n";

        pthread_mutex_lock(&pool->mutexBusy);   // busyNumThreads 专用的互斥锁
        pool->busyNumThreads--;     // 工作结束后--
        pthread_mutex_unlock(&pool->mutexBusy);
    }

    return NULL;


}




void ThreadPool::addTask(Task task)
{
  //  pthread_mutex_lock(& mutexPool);  queuue是线程安全



    if (this->shutdownThreadPool)
    {
 //       pthread_mutex_unlock(& mutexPool);
        return;
    }

    // 添加任务
    taskQ->addTask(task);


    pthread_cond_signal(& notEmpty);   // 唤醒消费者

 //   pthread_mutex_unlock(& mutexPool);


}



void ThreadPool::threadExit()
{
    pthread_t tid = pthread_self();

    for (int i = 0; i < this->maxNumThreads; i++)
    {
        if (threadIDs[i] == tid)
        {
            threadIDs[i] = 0;
            cout << "threadExit() called , " <<to_string(tid)  << " exiting --- \n";
            break;
        }
    }

    pthread_exit(NULL);  // 当前线程退出

}

int ThreadPool::getWorkNum()
{
    pthread_mutex_lock(& mutexPool);
    int workNum = liveNumThreads;   // 关闭线程

    pthread_mutex_unlock(&  mutexPool);
    return workNum;

}

int ThreadPool::getLiveNum()
{  pthread_mutex_lock(&  mutexBusy);

    int busyNum =   busyNumThreads;

    pthread_mutex_unlock(&  mutexBusy);
    return busyNum;
}

ThreadPool::~ThreadPool()
{

    pthread_mutex_lock(&  mutexPool);
    // 关闭线程池
    shutdownThreadPool=true;
    int liveNumThreads= liveNumThreads;
    pthread_mutex_unlock(& mutexPool);


    // 阻塞回收管理线程
    pthread_join( threadManagerID, NULL);

    // 唤醒阻塞的消费者线程ID
    for (int i = 0; i <  liveNumThreads; i++)
    {
        pthread_cond_signal(& notEmpty);  // 唤醒阻塞在 notEmpty 的线程
    }

    // 释放堆区
    if ( taskQ)
    {
        delete taskQ;
        taskQ=nullptr;

    }
    if ( threadIDs)
    {
       delete [] threadIDs;

    }

    // 释放互斥锁以及条件变量
    pthread_mutex_destroy(& mutexPool);
    pthread_mutex_destroy(& mutexBusy);

    pthread_cond_destroy(& notEmpty);



}

