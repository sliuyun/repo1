// 程序名：demo2.cpp，此程序用于演示socket服务端

#include<iostream>
#include<cstdio>
#include<cstring>
#include<string>
#include<netdb.h>           // 提供了存储服务端IP结构体的声明与相关函数
#include<stdio.h>			// 提供了C语言标准输入/输出函数
#include<stdlib.h>			// 提供了标准库函数，如malloc、free
#include<string.h>			// 提供了处理字符串的函数，如strlen
#include<unistd.h>			// 提供了与POSIX兼容的函数
#include<arpa/inet.h>		// 提供了与网络地址转换的函数与宏
#include<netinet/in.h>		// 提供了与网络通信协议相关的数据结构与宏定义
#include<sys/socket.h>		// 提供了套接字编程所需的所有定义和函数，包括套接字类型、协议、地址族
using namespace std;

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        cout<<"请依次输入程序名、端口号"<<endl;
        cout<<"例如:./demo2 8080";
        return -1;
    }

    // 第一步，创建监听套接字
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd == -1)
    {
        perror("socket");
        return -1;
    }

    // 第二步，把服务端用于通信的IP地址与端口绑定到监听套接字上
    struct sockaddr_in serveaddr;                   // 存储服务端IP地址与端口的结构体
    memset(&serveaddr,0,sizeof(serveaddr));
    serveaddr.sin_family = AF_INET;                 // 指定协议
    serveaddr.sin_port = htons(atoi(argv[1]));      // 指定端口号
    serveaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 指定IP地址
    if(bind(listenfd,(struct sockaddr*)&serveaddr,sizeof(serveaddr)) == -1)
    {
        perror("bind");
        close(listenfd);
        return -1;
    }

    // 第三步，把监听套接字变为被动套接字
    if(listen(listenfd,10) != 0)
    {
        perror("listen");
        close(listenfd);
        return -1;
    }

    // 第四步，等待客户端连接。如果客户端没有连接上来，accept()函数会阻塞程序执行
    int clientfd = accept(listenfd,0,0);
    if(clientfd == -1)
    {
        perror("accept");
        close(listenfd);
        return -1;
    }
    cout<<"客户端已连接\n";

    // 第五步，接收客户端发送的数据
    char buffer[1024];
    while(true)
    {
        int res;
        memset(buffer,0,sizeof(buffer));
        // 接收客户端的请求报文，如果客户端没有发送数据报文，recv函数将阻塞程序执行
        // 如果客户端已断开函数，那么recv函数返回0，否则返回接收到的数据报文的长度
        if((res = recv(clientfd,buffer,sizeof(buffer),0)) <=0 )
        {
            cout<<"res = "<<res<<endl;
            break;
        }
        cout<<"接收："<<buffer<<endl;

        strcpy(buffer,"ok");
        if((res = send(clientfd,buffer,strlen(buffer),0)) <= 0)
        {
            perror("send");
            break;
        }
        cout<<"发送："<<buffer<<endl;
    }

    // 第六步，关闭套接字
    close(listenfd);
    close(clientfd);
    return 0;

}