// 程序名：demo1.cpp，此程序用于演示socket客户端

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
    if(argc != 3)
    {
        cout<<"请依次输入程序名、服务端IP、服务端端口\n";
        cout<<"Example:./demo1 192.168.87.53 1234";
        return -1;
    }

    // 第一步，创建客户端的socket
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        perror("socket");
        return -1;
    }

    // 第二步，向服务器发起请求
    struct hostent* h;              // 用于存放服务端IP的数据结构
    if((h = gethostbyname(argv[1])) == 0)
    {
        cout<<"gethostbyname failed.\n";
        close(sockfd);
        return -1;
    }
    struct sockaddr_in serveaddr;
    memset(&serveaddr,0,sizeof(serveaddr));
    serveaddr.sin_family = AF_INET;             // 协议族
    serveaddr.sin_port = htons(atoi(argv[2]));  // 待连接的服务端的端口号
    memcpy(&serveaddr.sin_addr,h->h_addr,h->h_length);
    if(connect(sockfd,(struct sockaddr*)&serveaddr,sizeof(serveaddr)) == -1)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }
    

    // 第三步，与服务端通讯
    // 客户端发送一个请求报文给服务器，等待服务器的回复，收到回复后，在发送下一个请求报文
    char buffer[1024];
    for(int i = 0; i<3; i++)        // 发送3次      
    {
        int res;
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"hello world %d",i+1);       // 构造客户端发送的请求报文
        res = send(sockfd,buffer,sizeof(buffer),0);
        if(res == -1)
        {
            perror("send");
            close(sockfd);
            return -1;
        }

        memset(buffer,0,sizeof(buffer));
        // 接收服务端的回应报文，如果服务端没有发送回应报文，则recv函数将阻塞等待
        res = recv(sockfd,buffer,sizeof(buffer),0);
        if(res <= 0)
        {
            cout<<"res = "<<res<<endl;
            break;
        }
        cout<<"接收："<<buffer<<endl;
        sleep(1);
    }

    // 第四步，关闭socket，释放资源
    close(sockfd);
    return 0;
}