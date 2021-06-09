/**
* File Name: TaskQueue.cpp
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

#include "TaskQueue.h"


template<typename T>
TaskQueue<T>::TaskQueue()
{
    pthread_mutex_init(&m_mutex, NULL);

}

template<typename T>
TaskQueue<T> ::~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);

}

template<typename T>

void TaskQueue<T>::addTask(Task<T>  task_)
{
    pthread_mutex_lock(&m_mutex);
    m_taskQ.push(task_);
    pthread_mutex_unlock(&m_mutex);

}
template<typename T>

void TaskQueue<T>::addTask(callback f, void *arg)
{

    pthread_mutex_lock(&m_mutex);
    m_taskQ.push(Task<T>(f,arg));  // 包装后push
    pthread_mutex_unlock(&m_mutex);

}
template<typename T>

Task<T> TaskQueue<T>::taskTask()
{
    Task<T> t;
    if(!m_taskQ.empty())
    {
        t=m_taskQ.front();
        m_taskQ.pop();
    }
    return  t;
}

