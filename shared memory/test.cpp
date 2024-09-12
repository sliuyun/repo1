// 该程序演示共享内存的完整使用步骤

#include<iostream>
#include<cstring>
#include<sys/shm.h>
#include<sys/ipc.h>

#include<sys/types.h>
#include<unistd.h>
using namespace std;

struct figure{  // 人物类
    char name[50];
    size_t age;
    double height;
};

int main()
{
    // 第一步，创建共享内存
    int shmid = shmget(0x3339,sizeof(figure),0666 | IPC_CREAT);
    if(shmid == -1){
        cerr<<"创建共享内存失败。\n";
        return -1;
    }

    cout<<"shmid = "<<shmid<<endl;

    // 第二步，将共享内存连接到当前进程的地址空间
    figure* ptr = (figure*)shmat(shmid,0,0);
    if(ptr == (void*)-1){
        cerr<<"共享内存连接失败。\n";
        return -1;
    }
    cout<<"共享内存键值："<<shmid<<"\n";

    // 第三步，使用共享内存，读共享内存进行读/写操作
    cout<<"原共享内存数据：\n";
    cout<<"姓名："<<ptr->name<<"， 年龄："<<ptr->age<<"， 身高："<<ptr->height<<endl;

    // ptr->name = "SunWuKong"; // 这个操作是不合法的，因为不能对数组名赋值
    strcpy(ptr->name,"SunWuKong");
    ptr->age = 100;
    ptr->height = 200.00;
    cout<<"修改后共享内存数据：\n";
    cout<<"姓名："<<ptr->name<<"， 年龄："<<ptr->age<<"， 身高："<<ptr->height<<endl;

    int res = execl("/root/socket/C8/app1","app1",shmid,(char*)NULL);

    // 第四步，把共享内存从当前进程分离
    shmdt(ptr);


    // 第五步，删除共享内存
    if(shmctl(shmid,0,0) == -1)
    {
        cerr<<"删除共享内存失败。\n";
        return -1;
    }

}