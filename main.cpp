#include <iostream>

#include <pthread.h>
#include"ThreadPool.h"

#include <unistd.h>

void taskFunc(void *arg)
{
    //  int num=*(int*)arg; //转换后取值
    int num=*(int*)arg;

    cout<<"thread "<<pthread_self()<<" is working, number= "<<num<<endl;

    sleep(1);
}

int main()
{

    // 创建线程池
    ThreadPool  pool = ThreadPool(3, 10 );

    for (int i = 0; i < 100; i++)
    {
        int* num=new int(i+100);




        pool.addTask(Task(taskFunc,num));

    }

   // free(num);

    sleep(30);  // 睡眠30秒



    printf("-------exit---------------\n");

    return 0;
}
