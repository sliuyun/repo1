// 程序名：tcppoll.cpp，此程序用于演示socket服务端

#include<iostream>
#include<cstdio>
#include<cstring>
#include<string>
#include<poll.h>
#include<netdb.h>           // 提供了存储服务端IP结构体的声明与相关函数
#include<stdio.h>			// 提供了C语言标准输入/输出函数
#include<stdlib.h>			// 提供了标准库函数，如malloc、free
#include<string.h>			// 提供了处理字符串的函数，如strlen
#include<unistd.h>			// 提供了与POSIX兼容的函数
#include<arpa/inet.h>		// 提供了与网络地址转换的函数与宏
#include<netinet/in.h>		// 提供了与网络通信协议相关的数据结构与宏定义
#include<sys/socket.h>		// 提供了套接字编程所需的所有定义和函数，包括套接字类型、协议、地址族
using namespace std;

int initserve(int in_port);

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        printf("请依次输入程序名、端口号\nExample:./app 9999\n");
        return -1;
    }

    // 初始化用于监听的socket
    int listensock = initserve(atoi(argv[1]));
    printf("listenfd = %d\n",listensock);
    if(listensock < 0){return -1;}

    struct pollfd fds[1024];        // fds存放用于监视的socket

    // 初始化数组，把全部的socket设置为-1，如果数组中的socket值为-1，那么poll将忽略他们
    for(int i = 0; i<1024; i++){
        fds[i].fd = -1;
    }

    // poll监视listensock的读事件
    fds[listensock].fd = listensock;
    fds[listensock].events = POLLIN;            // POLLIN表示读事件，POLLOUT表示写事件

    int maxfd = listensock;                     // fds数组中需要监视的socket实际大小

    while(true)
    {
        // 调用poll()，等待事件的发生
        int infds = poll(fds,maxfd+1,10000);

        // 如果infds<0，表示调用poll失败
        if(infds<0)
        {
            perror("poll falied.\n");break;
        }

        // 如果infds==0，表示poll超时
        if(infds == 0)
        {
            printf("poll() timeout.\n");continue;
        }

        // 如果infds>0，表示有事件发生，infds存放了已发生事件的个数
        for(int eventfd = 0;eventfd<=maxfd; eventfd++)
        {
            if(fds[eventfd].fd < 0)continue;        // 如果fd为0，则忽略它

            if((fds[eventfd].revents&POLLIN) == 0)continue;       // 如果没有读事件，忽略它

            // 如果发生事件的是listensock，表示已连接队列中有准备好的sokcet（有新客户连接上来）
            if(eventfd == listensock)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(listensock,(struct sockaddr*)&client,&len);
                if(clientfd<0){ perror("accept() failed.\n");continue;}

                printf("accept client(socket = %d)\n",clientfd);

                // 修改fds数组中clientsock位置的元素
                fds[clientfd].fd = clientfd;
                fds[clientfd].events = POLLIN;

                if(maxfd<clientfd) maxfd = clientfd;    // 更新maxfd的值
            }
            else
            {
                // 如果是客户端连接的socket有事件，表示有报文发送过来了或连接已断开
                char buffer[1024];  // 存放从客户端读取的数据
                memset(buffer,0,sizeof(buffer));
                int res = recv(eventfd,buffer,sizeof(buffer),0);
                if(res <= 0)  // 对于res == 0 或 res < 0 的情况一起处理
                {
                    if(res == 0)
                        printf("client(eventfd = %d) disconnected.\n",eventfd);
                    else
                        perror("recv failed");

                    close(eventfd);         // 关闭客户端的socket
                    fds[eventfd].fd = -1;   // 修改fds数组中clientfd位置的元素，置为-1，poll将忽略该元素

                    // 重新计算maxfd的值
                    if(eventfd == maxfd)
                    {
                        for(int i = maxfd; i > 0; i--)        // 从后面往前找
                        {
                            if(fds[i].fd != -1)
                            {
                                maxfd = i;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    // 如果客户端有报文发送过来
                    printf("recv(eventfd = %d)%s\n",eventfd,buffer);

                    send(eventfd,buffer,sizeof(buffer),0);
                }
            }
        }
    }
}







int initserve(int in_port)
{
    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd == -1){
        perror("socket");
        return -1;
    }

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(in_port);
    saddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd,(sockaddr*)&saddr,sizeof(saddr)) == -1)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    // 设置socket不使用Nagel算法
    int opt = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    if(listen(sockfd,5) == -1)
    {
        perror("listen");
        close(sockfd);
        return -1;
    }

    return sockfd;
}


