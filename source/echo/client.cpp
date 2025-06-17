#include"../server.hpp"
int main()
{
    Socket sk;
    sk.CreateClient();
    char buf[]={"Hello Server!"};
    sk.NonBlockingSend(buf,sizeof(buf));
    char buf1[2024];
    sk.Recv(buf1,sizeof(buf1));
    std::cout<<buf1<<std::endl;
    int i=0;
    while(1)
    {
   
        sleep(1);
        DEBUG_LOG("第%d秒",i++);
    }
    return 0;
}