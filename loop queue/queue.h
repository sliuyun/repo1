// 循环队列类
#ifndef __QUEUE_H_
#define __QUEUE_H_


#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/sem.h>
using namespace std;

// 循环队列
template <class T,int MaxLength>
class squeue
{
private:
    bool initd; // 队列被初始化标志，true——已初始化，false——未初始化
    T data[MaxLength];  // 用数组存储循环队列中的元素
    int head;   // 队列的头指针
    int tail;   // 队列的尾指针
    int length; // 队列的实际长度
    squeue(const squeue& ) = delete;    // 禁用拷贝构造函数
    squeue& operator=(const squeue& ) = delete; // 禁用赋值运算符

public:

    // 构造函数
    squeue(){ init(); }

    // 循环队列的初始化函数
    void init()
    {
        if(initd != true)
        {
            head = 0;
            tail = MaxLength-1;
            length = 0;
            memset(data,0,sizeof(data));
            initd = true;
        }
    }


    // 判断循环队列是否为空，返回值：true-空，false-非空
    bool isempty()
    {
        if(length == 0) return true;
        return false;
    }

    // 判断循环队列是否已满，若已满：true-已满，false-未满
    bool isfull()
    {
        if(length == MaxLength) return true;
        return false;
    }    

    // 队尾元素入队
    // 元素入队，返回值：false—失败，true—成功
    bool push(const T& ee)
    {
        if(isfull() == true)
        {
            cout<<"循环队列已满。入队失败。\n";
            return false;
        }

        // 先移动对尾指针，然后在拷贝数据
        tail = (tail+1)%MaxLength;  // 对尾指针后移
        data[tail] = ee;
        length++;
        return true;
    }

    // 队头元素出队
    // 元素出队，返回值：false-失败，true-成功
    bool pop()
    {
        if(isempty() == true) return false;

        head = (head+1)%MaxLength;  // 队列头指针后移
        length--;
        return true;
    }

    // 求队列长度，返回值：>=0-队列中元素的个数
    int size()
    {
        return length;
    }



    // 查看队头元素的值，元素不出队
    T& front()
    {
        return data[head];
    }

    // 显示循环队列中的全部元素，从队头元素开始打印
    // 这是一个临时的用于调试的函数，队列中的元素的数据类型支持cout输出才可用
    void print()
    {
        for(int i = 0; i<length; i++)
        {
            cout<<"data["<<(head+i)%MaxLength<<"],value = "\
            <<data[(head+i)%MaxLength]<<endl;
        }
    }
};

#endif