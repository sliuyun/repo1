// 程序名：demo1.cpp，此程序用于演示socket通讯的服务端，并封装成ctcpserve类

#include<iostream>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
// #include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
using namespace std;

class ctcpserve
{
private:
    int listenfd;       // 服务端监听套接字，-1表示未初始化
    int clientfd;       // 通信套接字
    string ip;          // 客户端的ip
    unsigned short port;// 服务端服务端口

public:
    ctcpserve():listenfd(-1),clientfd(-1){}

    // int getlistenfd(){ return listenfd; }

    // 初始化服务端监听套接字
    bool initserve(const unsigned int in_port)
    {
        // 第一步，创建用于监听的套接字
        if((listenfd = socket(PF_INET,SOCK_STREAM,0)) == -1)
        {
            perror("socket");
            return false;
        }

        port = in_port;

        // 第二步，将服务端用于通信的IP和端口绑定到socket上
        struct sockaddr_in serveaddr;
        memset(&serveaddr,0,sizeof(serveaddr));
        serveaddr.sin_family = PF_INET;
        serveaddr.sin_port = htons(port);
        serveaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        // 绑定服务端的IP和端口
        if(bind(listenfd,(sockaddr*)&serveaddr,sizeof(serveaddr)) == -1)
        {
            close(listenfd);
            listenfd = -1;
            perror("bind");
            return false;
        }

        // 第三步，把socket设置为监听状态
        if(listen(listenfd,10) ==-1)
        {
            close(listenfd);
            listenfd = -1;
            perror("listen");
            return false;
        }

        return true;
    }

    // 第四步，等待客户端连接。如果客户端没有连接上来，accept()函数会阻塞程序执行
    bool accept()
    {
        // 受理客户端的连接
        struct sockaddr_in caddr;               // 存储客户端的结构体
        socklen_t addrlen = sizeof(caddr);
        if((clientfd = ::accept(listenfd,(sockaddr*)&caddr,&addrlen)) == -1)
        {
            perror("accept");
            close(listenfd);
            return false;
        }

        // 获取客户端的IP（字符串格式）
        ip = inet_ntoa(caddr.sin_addr);        
        return true;
    }

    // 获取客户端IP
    const string& getclientip() const
    {
        return ip;
    }

    // 向对端发送报文，成功返回true，失败返回false
    bool send(const string& buffer)
    {
        if(clientfd == -1) return false;

        if((::send(clientfd,buffer.c_str(),buffer.size(),0)) <= 0)
        {
            perror("send");
            close(listenfd);
            close(clientfd);
            listenfd = clientfd = -1;
            return false;
        }
        return true;
    }

    // 接收对端的数据，成功返回true，失败返回false
    // buffer-存放接收到的数据，maxlen-单次接收的最大字节数
    bool recv(string& buffer,int maxlen)
    {
        buffer.clear();
        buffer.resize(maxlen);
        int readn = ::recv(clientfd,&buffer[0],buffer.size(),0);
        if(readn == 0)
        {
            cout<<"客户端已断开连接\n";
            return false;
        }
        else if(readn < 0)
        { 
            perror("recv");
            close(listenfd);
            close(clientfd);
            listenfd = clientfd = -1;
            return false;
        }
        buffer.resize(readn);

        return true;        
    }


    ~ctcpserve()
    {
        if(listenfd != -1){ close(listenfd); }
        if(clientfd != -1){ close(clientfd); }
    }
};

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        cout<<"请依次输入程序名、服务端端口\n";
        cout<<"Example:./serve 9999\n";
        return -1;
    }
    ctcpserve cs;

    cs.initserve(atoi(argv[1]));

    // 死循环，会一直等待客户端连接。如果客户端没有连接上来，accept()函数会阻塞程序执行
    if(cs.accept() !=false)
    {
        cout<<"客户端IP："<<cs.getclientip()<<endl;        
    }
    // 当两个程序在同一台电脑上跑，且客户端输入的是服务端的主机名或域名而不是IP时，客户端会使用回环地址与服务端通信


    // 接收客户端发送的数据
    string buffer;
    while(true)
    {
        int res;
        // 接收客户端的请求报文，如果客户端没有发送数据报文，recv函数将阻塞程序执行
        // 如果客户端已断开函数，那么recv函数返回0，否则返回接收到的数据报文的长度
        if((res = cs.recv(buffer,1024)) == false )
        {
            cout<<"res = "<<res<<endl;
            break;
        }
        cout<<"接收："<<buffer<<endl;

        buffer = "ok";
        if(cs.send(buffer) == false)
        {
            break;
        }
        cout<<"发送："<<buffer<<endl;
    }
}