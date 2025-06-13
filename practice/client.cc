#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<error.h>
#include<stdio.h>
#include"../server.hpp"
// int main()
// {
//     int fd=socket(AF_INET,SOCK_STREAM,0);
//     struct sockaddr_in addr_in;
//     addr_in.sin_family=AF_INET;
//     addr_in.sin_port=htons(8080);
//     addr_in.sin_addr.s_addr=inet_addr("127.0.0.1");
//     if(connect(fd,(const struct sockaddr*)&addr_in,sizeof(addr_in))==-1)
//     {
//         perror("Connect Error!");
//     }
//     char s[]="hello server!\n";
//     ssize_t ret=send(fd,s,sizeof(s),0);
//     if(ret==-1)
//     {
//         perror("Send Error!");
//     }
//     return 0;
// }
void SocketClient()
{
    Socket client;
    int fd=client.CreateClient();
    // 等待服务器准备好
    char ch[]="Hello Server!";
    if(send(fd,ch,sizeof(ch),0)==-1){ERROR_LOG("Write Error");abort();}
    printf("Write Content: %s\n",ch);
}
int main()
{
    SocketClient();
}