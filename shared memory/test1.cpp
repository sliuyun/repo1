// 该test1.cpp 用于简单演示创建共享内存并分离

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

int main(int argc,char* argv[])
{
    // 连接共享内存
    figure* point = (figure*)shmat(atoi(argv[1]),0,0);
    if(point == (void*)-1){
        cout<<"连接共享内存失败。\n";
        return -1;
    }

    cout<<"修改后共享内存数据：\n";
    cout<<"姓名："<<point->name<<"， 年龄："<<point->age<<"， 身高："<<point->height<<endl;

    // 把共享内存从当前进程分离
    shmdt(point);
}