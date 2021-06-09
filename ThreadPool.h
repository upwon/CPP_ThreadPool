/**
* File Name: ThreadPool.h
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

#ifndef CPP_THREADPOOL_THREADPOOL_H
#define CPP_THREADPOOL_THREADPOOL_H


#include"TaskQueue.h"

class ThreadPool
{
public:


// 创建线程池  构造函数
    ThreadPool(int minNumThreads, int maxNumThreads_ );


// 销毁线程池 析构函数
    ~ThreadPool();


// 给线程池添加任务
    void addTask(Task task);

// 取出一个任务
//  Task takeTask();

// 获取线程池中的工作线程
    int getWorkNum();

// 获取线程池中活着的线程池
    int getLiveNum();

private:

    [[noreturn]] static  void *worker(void *arg);

   static void *manager(void *arg);

    void threadExit();



private:
    // 任务队列
    TaskQueue *taskQ;

    // 管理者线程与工作线程
    pthread_t threadManagerID;    // 管理者线程ID
    pthread_t *threadIDs;    //工作线程ID

    int minNumThreads; // 最小线程数量
    int maxNumThreads; // 最大线程数量
    int busyNumThreads; // 忙的线程数量
    int liveNumThreads; // 存活的线程个数
    int destroyNumThreads; // 要销毁的线程个数

    // 互斥锁
    pthread_mutex_t mutexPool; //锁整个线程
    pthread_mutex_t mutexBusy; //专用于锁busyNumThreads

    // 条件变量
 //   pthread_cond_t notFull; // 任务队列是不是满了
    pthread_cond_t notEmpty; // 任务队列是不是空了

    // 销毁线程标志
    bool shutdownThreadPool; //true表示销毁线程


    static const int NUMBER=2;


};


#endif //CPP_THREADPOOL_THREADPOOL_H
