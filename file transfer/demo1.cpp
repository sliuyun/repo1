// 程序名：demo1.cpp，用于演示客户端


#include<iostream>
#include<fstream>
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

    // 用于传输字符串的send函数
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


    // 用于传输文件结构体的send函数
    bool send(void* buffer,const size_t size)
    {
        if(clientfd == -1) {  return false;  }

        int res = ::send(clientfd,buffer,size,0);
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

    // 向服务端发送文件内容
    bool sendfile(const string& filepath,const size_t filesize)
    {
        ifstream fin(filepath,ios_base::binary | ios_base::in);
        if(fin.is_open() == false){ cout<<"open："<<filepath<<" falied.\n"; return false; }

        int onread;              // 每次应读取的字节数
        int totalbytes = 0;     // 已读取的字节数
        char buffer[4096];      // 设定每次读取7块砖

        while(true)
        {
            memset(buffer,0,sizeof(buffer));

            // 计算本次应读取的字节数
            if(filesize-totalbytes > 4096) onread = 4096;
            else onread = filesize-totalbytes;

            // 从文件中读取数据
            fin.read(buffer,onread);                                 // 将数据从fin文件流读取到buffer缓冲区                                                        

            if(send(buffer,onread) == false){ return false; }        // 将数据从buffer缓冲区发送到clientfd套接字（因为封装的缘故，不需要clientfd这个参数）

            // 计算已读取的字节数
            totalbytes += onread;

            if(totalbytes == filesize) break;
        }

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
    if(argc != 5)
    {
        cout<<"请依次输入程序名、服务端IP、服务端的端口、文件路径、文件大小\n";
        cout<<"Example:./app1 DESKTOP-F3ILI7N 9999 abc.txt 1024\n";
        return -1;
    }

    // 创建套接字对象并向服务器发起连接
    ctcpclient cl;
    if(cl.connect(argv[1],atoi(argv[2])) == false)
    {
        return -1;
    }

    // 下面是发送文件的流程

    // 1.把待传输的文件名和大小告诉服务器

    // 文件结构体，因为我们传参时send函数的形参时void*，所以文件结构体可以放到main块内部
    struct fileinfo
    {
        char filename[100];      // 文件路径
        int filesize;           // 文件大小
    }testfile;
    memset(&testfile,0,sizeof(testfile));
    strcpy(testfile.filename,argv[3]);
    testfile.filesize = atoi(argv[4]);
    if(cl.send(&testfile,sizeof(fileinfo)) == false)
    {
        return -1;
    }

    // 2.等待服务端的确认报文（文件名和文件大小的确认）
    string buffer;
    if(cl.recv(buffer,2) == false){ return -1; }
    if(buffer != "ok"){ cout<<"服务端没有回复ok\n"; return -1; }

    // 3.发送文件内容
    if(cl.sendfile(testfile.filename,testfile.filesize) == false){
        return -1;
    }

    // 4.等待服务端的确认报文（服务端已接收完毕）
    if(cl.recv(buffer,2) == false){ return -1; }
    if(buffer != "ok") { cout<<"发送文件内容失败\n"; return -1; }
    cout<<"发送文件成功\n";

    return 0;
}






// 如果在命令行只想输入文件路径，那么就需要再main内部使用这个函数获取文件大小
int getfilesize(const string& str)
{    
    // 使用文件流时，必须要包含fstream文件，不然会报出“不允许使用不完整的类型  std::ofstream”
    // 打开文件
    ifstream fin(str);
    if(fin.is_open() == false)
    {
        cout<<"open "<<str<<" failed.\n";
        return -1;
    }


    // 先将文件指针移至末尾
    fin.seekg(0,ios_base::end);

    // 再获取文件大小
    int res = fin.tellg();

    // 关闭文件流
    fin.close();

    return res;
}