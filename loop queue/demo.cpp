
// 队列的测试程序
#include"queue.h"
// using namespace std;

int main()
{
    using ElemenType = int;

    squeue<ElemenType,10> s1;
    for(int i = 10; i<120; i+=10)
    {
        s1.push(i);
    }

    cout<<"队列的元素长度："<<s1.size()<<"\n";

    // 打印队列
    s1.print();

    return 0;
}