// 多进程的生产消费者模型的消费者模型

#include"head.h"

int main()
{
    struct figure
    {
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
        cout<<"shmat() failed.\n";
        return -1;
    }

    QQ->init(); // 初始化循环队列

    ElementType ee;

    csemp mutex;    mutex.init(0x5005);     // 用于给共享内存加锁
    csemp cond;     cond.init(0x5005,0,0);  // 信号量的值用于表示队列中数据元素的个数

    while(true)
    {
        mutex.wait();           // 加锁
        while(QQ->isempty())
        {
            mutex.post();       // 解锁
            cond.wait();        // 等待生产者的唤醒信号
            mutex.wait();       // 加锁
        }

        // 数据元素出队
        ee = QQ->front();   QQ->pop();
        mutex.post();           // 解锁

        // 处理出队的数据（把数据消除掉）
        cout<<"age = "<<ee.age<<", name = "<<ee.name<<endl;
        usleep(100);            // 假设处理数据需要时间，方便演示
    }

}