#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include"../server.hpp"
#define BLOCKSIZE 128
#define BUFFERSIZE 1280
// int main()
// {
//     int listen_fd=socket(AF_INET,SOCK_STREAM,0);
//     if(listen_fd==-1)
//     {
//         perror("Create Socket Error!");
//     }
//     // flag=0默认协议
//     //监听
//     struct sockaddr_in addr_in;
//     memset(&addr_in,0,sizeof(addr_in));
//     addr_in.sin_family=AF_INET;
//     addr_in.sin_port=htons(8080);
//     addr_in.sin_addr.s_addr=inet_addr("0.0.0.0");
//     if(bind(listen_fd,(const struct sockaddr*)&addr_in,sizeof(addr_in))==-1)
//     {
//         perror("Bind Address Error!");
//     }
//     if(listen(listen_fd,BLOCKSIZE)==-1)
//     {
//         perror("Listen  Error!");
//     }
//     struct sockaddr_in client_addr_in;
//     memset(&client_addr_in,0,sizeof(client_addr_in));
//     socklen_t len;
//     // 阻塞等待 flag==0
//     int fd=accept(listen_fd,(struct sockaddr*)&client_addr_in,&len);
//     if(fd==-1)
//     {
//         perror("Accept Error!");
//     }
//     char buff[BUFFERSIZE];
//     //阻塞读取
//     ssize_t  ret=recv(fd,buff,sizeof(buff),0);
//     if(ret==-1)
//     {
//         perror("Read Error!");
//     }
//     printf("Read Content: %s",buff);
// }

void SocketServer()
{
    Socket server;
    int fd=server.CreateServer();

    char buff[128];
    if(read(fd,buff,sizeof(buff))==-1){ERROR_LOG("Read Error");abort();}
    printf("Read Content: %s\n",buff);
}
int main()
{
    SocketServer();
    return 0;
}