// 程序名：tcpselect.cpp，用于演示select模型

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

int initserve(int in_port);

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        cout<<"请依次输入程序名、端口号\n";
        cout<<"Example:./app 5005\n";
    }

    // 初始化服务端用于监听的套接字
    int listenfd = initserve(atoi(argv[1]));
    cout<<"listenfd = "<<listenfd<<endl;

    if(listenfd == -1)
    {
        cout<<"initserve() failed.\n";
        return -1;
    }

// 网络通讯-读事件
// 1. 已连接队列中有已准备的socket（有新的客户端连上来）
// 2. 接受缓存中有数据可以读（对端发送的报文已到达）
// 3. TCP连接已断开（对端调用close()函数关闭了连接）

// 网络通讯-写事件
// ​发送缓冲区没有满，可以写入数据（可以向对端发送报文）

    fd_set readfds;             // 需要监视读事件的socket集合，大小一般为1024位
    FD_ZERO(&readfds);          // 初始化readfds，把bitmap中的每一位都置为0
    FD_SET(listenfd,&readfds);  // 把服务端用于监听的socket加入readfds 

    int maxfd = listenfd;       // readfds中的socket的最大值

    while(true)
    {
        // 超时时间的结构体
        struct timeval timeout; 
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        // select()等待事件的发生

        fd_set temfds = readfds;        // 在select()函数中，会修改bitmap，所以要先把readfds复制一份，再把temfs传给select()函数
        // int infds = select(maxfd+1,&temfds,NULL,NULL,&timeout);
        int infds = select(maxfd+1,&temfds,NULL,NULL,0);

        if(infds < 0)
        {
            perror("select failed.\n");
            break;
        }

        // 如果infds==0，表示select超时
        if(infds == 0)
        {
            perror("select() timeout.\n");
            break;
        }

        // 如果infds>0，表示有事件发生，infds存放了已发生事件的个数
        for(int eventfd = 0; eventfd<=maxfd; eventfd++)
        {
            if(FD_ISSET(eventfd,&temfds) == 0) continue;        // 如果eventfd在bitmap中的标志为0，表示它没有事件，continue

            // 如果发生事件的是listenfd，表示已连接队列中已有准备好的socket（有新客户连接上来）
            if(eventfd == listenfd)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(listenfd,(struct sockaddr*)&client,&len);
                if(clientfd < 0){ perror("accept() falied.\n"); continue; }

                printf("accept client(socket = %d) ok.\n",clientfd);

                FD_SET(clientfd,&readfds);                      // 把bitmap中新连上来的客户端的标志设置为1

                if(maxfd < clientfd) maxfd = clientfd;          // 更新maxfd的值
            }
            else
            {
                // 如果是客户端连接的socket有事件，表示接收缓存中有数据可以读（对端发送的报文已送达），或者有客户已断开连接
                char buffer[1024];              // 存放从客户端读取的数据
                memset(buffer,0,sizeof(buffer));
                if(recv(eventfd,buffer,sizeof(buffer),0) <= 0)
                {
                    // 如果客户端的连接已断开
                    printf("client(eventfd = %d) disconencted.\n",eventfd);
                    close(eventfd);     // 关闭客户端的socket
                    FD_CLR(eventfd,&readfds);   // 把bitmap中已关闭客户端的标志位清空
                    if(eventfd == maxfd)
                    {
                        for(int i = maxfd; i>0; i--)
                        {
                            if(FD_ISSET(i,&readfds))
                            {   
                                maxfd = i;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    // 如果客户端有报文发过来
                    printf("recv(eventfd = %d).%s\n",eventfd,buffer);

                    // 把接收到的报文内容原封不动的发回去
                    send(eventfd,buffer,sizeof(buffer),0);
                }
            }
        }
    }
    return 0;
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

