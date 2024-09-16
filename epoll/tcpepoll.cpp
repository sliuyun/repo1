// 程序名：tcpepoll.cpp，该程序用于演示epoll模型实现网络通讯的服务端

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/fcntl.h>
#include<sys/epoll.h>

// 用于初始化服务端监听socket的函数
int initserve(int in_port);

int main(int argc,char* argv[])
{
    if(argc != 2){
        printf("请依次输入程序名、端口号\nExample:./app 9999\n");
        return -1;
    }

    // 先初始化用于监听的socket
    int listenfd = initserve(atoi(argv[1]));
    printf("listenfd = %d\n",listenfd);
    if(listenfd < 0)return -1;

    // 创建epoll句柄
    // 该函数会返回一个句柄，就是文件描述符。参数在Linux2.6.8之后就没有意义，任填一个大于0的树即可
    int epollfd = epoll_create(1);

    // 为服务端的listenfd准备读事件
    struct epoll_event ev;          // 声明事件的数据结构
    ev.data.fd = listenfd;          // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回
    ev.events = EPOLLIN;            // 打算让epoll监视listenfd的读事件

    epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&ev);      // 把需要监视的socket加入epollfd中

    struct epoll_event evs[10];     // 存放epoll返回的事件

    while(true)
    {
        // 等待监视的socket有事件发生
        int infds = epoll_wait(epollfd,evs,sizeof(evs)/sizeof(epoll_event),-1);

        // 如果返回失败
        if(infds < 0)
        {
            perror("eopll() failed");break;
        }

        // 超时
        if(infds == 0)
        {
            printf("epoll() timeout.\n");continue;
        }

        // 如果infds>0，表示有事件发生的socket数量
        // 遍历epoll返回的数组evs
        for(int i = 0; i<infds; i++)    
        {
            // printf("socket = %d,events = %d\n",evs[i].data.fd,evs[i].events);

            // 如果发生事件的是listenfdm，表示有新的客户端连接上来
            if(evs[i].data.fd == listenfd)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(listenfd,(struct sockaddr*)&client,&len);

                printf("accept client(socket = %d) ok.\n",clientfd);

                // 为新客户端准备可读事件，并添加到epoll中
                ev.data.fd = clientfd;
                ev.events = EPOLLIN;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&ev);
            }
            else
            {
                // 如果是客户端连接的socket有事件，表示有报文发过来或者连接已断开
                char buffer[1024];              // 存放从客户端读取的数据
                memset(buffer,0,sizeof(buffer));
                if(recv(evs[i].data.fd,buffer,sizeof(buffer),0) <= 0)
                {
                    // 如果客户端的连接已断开
                    printf("client (eventfd = %d) disconnected.\n",evs[i].data.fd);
                    close(evs[i].data.fd);

                    // 从epoll中删除客户端的socket，如果socket被关闭，会自动从epollfd中删除，所以，以下代码不必启用
                    // epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[i].data.fd,0);
                }
                else
                {
                    // 如果客户端有报文发送过来
                    printf("recv (eventfd = %d):%s\n",evs[i].data.fd,buffer);

                    // 把接受到的数据原封不动发送回去
                    send(evs[i].data.fd,buffer,strlen(buffer),0);
                }
            }
        }
    }
    return 0;
}


int initserve(int in_port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(in_port);
    saddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr)) == -1)
    {
        perror("bind");
        return -1;
    }

    // 不使用Nagel算法
    int opt = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    if(listen(sockfd,10) == -1)
    {
        perror("listen");
        return -1;
    }

    return sockfd;    
}