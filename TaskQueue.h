/**
* File Name: TaskQueue.h
* Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
*
* Author: Xianwen Wang
* Version: v1.0
* Date: 2021/6/8 20:28
* LastEditTime: 2021/6/8 20:28
* Copyright: Xianwen Wang
* Project Name: CPP_ThreadPool
* IDE: CLion
* Reference:
* History: 修改历史记录列表， 每条修改记录应包括修改日期、修改者及修改内容简述
*/
//
#include<queue>
#include <pthread.h>

using namespace std;

#ifndef CPP_THREADPOOL_TASKQUEUE_H
#define CPP_THREADPOOL_TASKQUEUE_H

using callback = void (*)(void *arg);

// 任务结构体
struct Task
{
    // 无参构造
    Task()
    {
        functions = nullptr;
        arg = nullptr;
    }

    // 有参构造
    Task(callback f, void *arg)
    {
        this->functions = f;
        this->arg = arg;
    }

    // void (*functions)(void *arg);
    callback functions;
    void *arg;
};

class TaskQueue
{
public:
    TaskQueue();

    ~TaskQueue();

    // 添加任务
    void addTask(Task task_);
    void addTask(callback f,void * arg);

    // 取出一个任务
    Task taskTask();

    // 获取当前任务的个数
    inline int getTaskNumber()
    {
        return m_taskQ.size();
    }


private:
    queue<Task> m_taskQ; // 动态的容器 队列

    // 互斥锁
    pthread_mutex_t m_mutex;


};


#endif //CPP_THREADPOOL_TASKQUEUE_H
