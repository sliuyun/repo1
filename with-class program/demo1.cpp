// 程序名：demo1.cpp，此程序用于演示socket通讯的客户端，并封装成ctcpclient类

#include<iostream>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
using namespace std;

class ctcpclient        // TCP通讯的客户端
{
private:
    int clientfd;       // 客户端的套接字。-1表示未连接或连接已断开，大于等于0表示有效的socket
    string ip;          // 服务端的IP/域名/主机名
    unsigned short port;// 服务端的服务端口

public:
    ctcpclient():clientfd(-1){}

    int getsock(){ return clientfd; }

    // 向服务端发起连接请求，成功返回true，失败返回false
    bool connect(const string& in_ip,const unsigned short in_port)
    {
        if(clientfd != -1){ return false; }     // 如果已连接，那么不允许二次连接

        ip = in_ip; port = in_port;             // 把服务端的IP和端口保存到成员变量中

        // 第一步，创建客户端的socket
        clientfd = socket(AF_INET,SOCK_STREAM,0);
        if(clientfd == -1)
        {
            perror("socket");
            return false;
        }

        // 第二步，向服务器发起请求
        struct hostent* h;              // 用于存放服务端IP的数据结构
        if((h = gethostbyname(ip.c_str())) == nullptr)
        {
            cout<<"gethostbyname failed.\n";
            close(clientfd);
            clientfd = -1;              // 关闭sock只是关闭文件流，但clientfd这个成员变量任然存在，需要置为-1
            return false;
        }
        struct sockaddr_in serveaddr;
        memset(&serveaddr,0,sizeof(serveaddr));
        serveaddr.sin_family = AF_INET;             // 协议族
        serveaddr.sin_port = htons(port);           // 待连接的服务端的端口号
        memcpy(&serveaddr.sin_addr,h->h_addr,h->h_length);
        if(::connect(clientfd,(struct sockaddr*)&serveaddr,sizeof(serveaddr)) == -1)
        {
            perror("connect");
            close(clientfd);
            clientfd = -1;
            return false;
        }

        return true;

    } 

    bool send(const string& buffer)
    {
        if(clientfd == -1) {  return false;  }

        int res = ::send(clientfd,buffer.c_str(),buffer.size(),0);
        if(res == -1)
        {
            close(clientfd);
            clientfd = -1;
            perror("send");
            return false;
        }

        return true;        
    }

    // 接收服务端的报文，成功返回true，失败返回false
    // buffer-存放接收到的报文内容，maxlen-本次接收报文的最大长度
    bool recv(string& buffer,int maxlen)
    {
        buffer.clear();             // 清空容器
        buffer.resize(maxlen);      // 设置容器的大小为maxlen
        int readn = ::recv(clientfd,&buffer[0],buffer.size(),0);
        if(readn < 0)
        {
            close(clientfd);
            clientfd = -1;
            perror("recv");
            return false;
        }
        buffer.resize(readn);
        return true;
    }


    ~ctcpclient()
    {
        // 如果程序不发生错误，则由析构函数关闭套接字资源
        if( clientfd != -1){ close(clientfd); }
    }
};



int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        cout<<"请依次输入程序名、服务端IP、服务端的端口\n";
        cout<<"Example:./app1 DESKTOP-F3ILI7N 9999\n";
        return -1;
    }

    ctcpclient cl;
    cl.connect(argv[1],atoi(argv[2]));

    // 与服务端通讯
    // 客户端发送一个请求报文给服务器，等待服务器的回复，收到回复后，在发送下一个请求报文
    string buffer;
    for(int i = 0; i<10; i++)        // 发送3次      
    {
        bool res;
        buffer = "hello world "+to_string(i+1);       // 构造客户端发送的请求报文
        if((cl.send(buffer)) == false)
        {
            break;
        }

        // 接收服务端的回应报文，如果服务端没有发送回应报文，则recv函数将阻塞等待
        res = cl.recv(buffer,1024);
        if(res == false)
        {
            break;
        }
        cout<<"接收："<<buffer<<endl;
        sleep(1);
    }

    return 0;
}