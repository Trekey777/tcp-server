#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

// 1. 匿名管道（Pipe）
void demo_pipe() {
    printf("----- Anonymous Pipe Demo -----\n");
    int fd[2];
    pipe(fd);  // 创建管道

    if (fork() == 0) {  // 子进程：写入数据
        close(fd[0]);   // 关闭读端
        write(fd[1], "Hello from Pipe!", 16);
        close(fd[1]);
        exit(0);
    } else {            // 父进程：读取数据
        close(fd[1]);   // 关闭写端
        char buf[16];
        read(fd[0], buf, sizeof(buf));
        printf("Received: %s\n", buf);
        close(fd[0]);
    }
}

// 2. 命名管道（FIFO）
void demo_fifo() {
    printf("\n----- Named Pipe (FIFO) Demo -----\n");
    mkfifo("/tmp/myfifo", 0666);  // 创建命名管道

    if (fork() == 0) {  // 子进程：写入数据
        int fd = open("/tmp/myfifo", O_WRONLY);
        write(fd, "Hello from FIFO!", 16);
        close(fd);
        exit(0);
    } else {            // 父进程：读取数据
        int fd = open("/tmp/myfifo", O_RDONLY);
        char buf[16];
        read(fd, buf, sizeof(buf));
        printf("Received: %s\n", buf);
        close(fd);
        unlink("/tmp/myfifo");  // 删除管道文件
    }
}


void demo_msg_queue() {
    printf("\n----- Message Queue Demo -----\n");
    key_t key = ftok("/tmp", 'B');
    int msqid = msgget(key, 0666 | IPC_CREAT);

    if (fork() == 0) {  // 子进程：发送消息
        struct msgbuf msg = {1, "Hello from Message Queue!"};
        msgsnd(msqid, &msg, strlen(msg.mtext)+1, 0);
        exit(0);
    } else {            // 父进程：接收消息
        struct msgbuf msg;
        msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0);
        printf("Received: %s\n", msg.mtext);
        msgctl(msqid, IPC_RMID, NULL);  // 删除消息队列
    }
}

// 4. 共享内存（Shared Memory）
void demo_shared_memory() {
    printf("\n----- Shared Memory Demo -----\n");
    int shmid = shmget(IPC_PRIVATE, 1024, 0666 | IPC_CREAT);
    char *shm = (char*)shmat(shmid, NULL, 0);

    if (fork() == 0) {  // 子进程：写入数据
        sprintf(shm, "Hello from Shared Memory!");
        shmdt(shm);
        exit(0);
    } else {            // 父进程：读取数据
        wait(NULL);     // 等待子进程完成
        printf("Received: %s\n", shm);
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);  // 释放共享内存
    }
}

int main() {
    demo_pipe();        // 匿名管道
    demo_fifo();        // 命名管道
    demo_msg_queue();   // 消息队列
    demo_shared_memory(); // 共享内存
    return 0;
}