// 多进程的生产消费者模型的生产程序

#include"head.h"

int main()
{
    struct figure{
        char name[50];  // 姓名
        int age;        // 年龄
    };

    using ElementType = figure;

    // 初始化共享内存
    int shmid = shmget(0x5005,sizeof(squeue<ElementType,5>),0640 | IPC_CREAT);
    if(shmid == -1)
    {
        cout<<"shmget(0x5005) failed.\n";
        return -1;
    }

    // 把共享内存连接到当前进程的地址空间
    squeue<ElementType,5>* QQ = (squeue<ElementType,5>*)shmat(shmid,0,0);
    if(QQ == (void*)-1)
    {
        cout<<"shmat failed.\n";
        return -1;
    }

    QQ->init();

    ElementType ee;     // 创建一个数据元素

    csemp mutex; mutex.init(0x5001);    // 用于给共享内存加锁
    csemp cond;  cond.init(0x5002,0,0); // 信号量的值用于表示队列中的数据元素的个数

    mutex.wait();       // 加锁

    // 生产3个数据
    ee.age = 30; strcpy(ee.name,"caocao");  QQ->push(ee);
    ee.age = 45; strcpy(ee.name,"sunquan"); QQ->push(ee);
    ee.age = 50; strcpy(ee.name,"liubei");  QQ->push(ee);
    mutex.post();   // 解锁
    cond.post(3);   // 实参是3，表示生产3个数据

    shmdt(QQ);      // 把共享内存从当前进程中分离

}