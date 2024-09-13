// 程序名：demo2.cpp，用于演示服务端

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<unistd.h> 
#include<netdb.h>
#include<signal.h>
#include<libgen.h>  // 提供了从文件路径中获取文件名的函数的原型
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

        // 获取客户端的IP，并写入数据成员ip（字符串格式）
        ip = inet_ntoa(caddr.sin_addr);        
        return true;
    }

    // 获取客户端IP
    const string getclientip() const
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

    // 接收字符串数据
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

    // 接收文件结构体数据
    // 接收对端的数据，成功返回true，失败返回false
    // buffer-存放接收到的数据，maxlen-单次接收的最大字节数
    bool recv(void* buffer,const size_t size)
    {
        int readn = ::recv(clientfd,buffer,size,0);
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

        return true;        
    }

    bool recvfile(const string& filepath,const size_t filesize)
    {
        ofstream fout(filepath,ios_base::binary | ios_base::out | ios_base::trunc);
        if(fout.is_open() == false){
            cout<<"open: "<<filepath<<" falied.\n";
            return false;
        }

        int onread = 0;             // 单次读取的字节数
        int totalbytes = 0;         // 已读取的字节数
        char buffer[4096];             // 接收文件的缓冲区

        while(true)
        {
            if(filesize-totalbytes>4096) onread = 4096;
            else  onread = filesize-totalbytes;

            // 接收文件内容
            if(recv(buffer,onread) == false) return false;                  // 将数据从clientfd套接字读取到buffer缓冲区

            
            fout.write(buffer,onread);                                      // 将数据从buffer缓冲区读取到fout文件流

            // 计算已接收数据的字节数
            totalbytes += onread;

            if(totalbytes == filesize) break;
        }

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
    if(argc != 3)
    {
        cout<<"请依次依次输入程序名、通讯端口、文件存放的目录\n";
        cout<<"Example:./app2 9999 /root/sokcet";
    }
    /*三个概念：文件存放的目录 + 文件名 = 文件路径*/



    // 初始化服务端套接字
    ctcpserve cs;
    if(cs.initserve(atoi(argv[1])) == false)
    {
        return -1;
    }

    // 等待客户端发起请求
    if(cs.accept() == false)
    {
        return -1;
    }

    cout<<"客户端ip："<<cs.getclientip()<<endl;

    // 下面是接收文件的流程

    // 1.接收文件名和文件大小的信息
    // 文件结构体
    struct fileinfo
    {
        char filename[100];      // 文件路径
        int filesize;           // 文件大小
    }testfile;
    memset(&testfile,0,sizeof(testfile));
    if(cs.recv(&testfile,sizeof(fileinfo)) == false)
    {
        return -1;
    }
    cout<<"原文件结构体信息：\n";
    cout<<"文件路径："<<testfile.filename<<"\n";
    cout<<"文件大小："<<testfile.filesize<<"\n";

    // 重置文件目录，设置为服务端程序指定的存放路径
    char newfilepath[100] = {0};
    // strcpy(newfilepath,string(argv[2])+"/"+testfile.filename);
    strcat(newfilepath,argv[2]);
    strcat(newfilepath,"/");
    strcat(newfilepath,basename(testfile.filename));
    strcpy(testfile.filename,newfilepath);
    cout<<"修改后文件结构体信息：\n";
    cout<<"文件路径："<<testfile.filename<<"\n";
    cout<<"文件大小："<<testfile.filesize<<"\n";



    // 2.给客户端回复服务报文，表示客户端可以发送文件了
    if(cs.send("ok") == false)
    {
        return -1;
    }

    // 3.接收文件内容
    if(cs.recvfile(testfile.filename,testfile.filesize) == false)
    {
        cout<<"接收文件失败\n";
        return -1;
    }
    cout<<"接收文件内容成功\n";
     
    // 4.给客户端回复确认报文，表示文件已接收成功
    cs.send("ok");


    return 0;
}