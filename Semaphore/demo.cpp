// 测试程序，本程序显示用信号量给共享内存加锁
#include "head.h"
#include<iostream>

struct figure{
    int age;    // 年龄
    char name[50];  // 姓名
};

/*
查看信号量：ipcs -s         删除信号量：ipcrm sem 信号量id
查看共享内存：ipcs -m       删除共享内存：ipcrm -m 共享内存id 


*/


int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        cout<<"Example:./demo sunquan 40\n";
        return -1;
    }

    // 第一步，创建/获取共享内存，键值key为0x5005，也可以为其他的值
    int shmid = shmget(0x5005,sizeof(figure),0640 | IPC_CREAT);
    if(shmid == -1){
        cout<<"shmget(0x5005) failed.\n";
        return -1;
    }

    // 第二部，把共享内存连接到档期啊进程的地址空间
    figure* ptr = (figure*)shmat(shmid,0,0);
    if(ptr == (void*)-1){
        cout<<"shmat falied.\n";
        return -1;
    }


    // 初始化二元信号量
    csemp mutex;
    if(mutex.init(0x5005) == false){
        cout<<"mutex.init(0x5005) failed.\n";
        return -1;
    }

    cout<<"申请加锁……\n";
    mutex.wait();   // 申请加锁
    cout<<"加锁成功……\n";


    // 第三步，对共享内存进行读/写
    cout<<"共享内存的初始值：\n";
    cout<<"姓名："<<ptr->name<<", 年龄: "<<ptr->age<<"\n";
    ptr->age = atoi(argv[2]);
    strcpy(ptr->name,argv[1]);
    cout<<"写入后：\n";
    cout<<"姓名："<<ptr->name<<", 年龄: "<<ptr->age<<"\n";
    sleep(10);

    mutex.post();   // 解锁
    cout<<"解锁.\n";


    // 第四步，将共享内存从当前进程分离
    shmdt(ptr);

    // 第五步，删除共享内存
    int res = shmctl(shmid,IPC_RMID,0);
    if(res == -1){
        cout<<"shmclt falied.\n";
        return -1;
    }




}