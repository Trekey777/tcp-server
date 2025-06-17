//     char ch2[10]={0};
//     buffer1.ReadBufferAndPop(ch2,4);
//     printf("ch2: %s",ch2);
//     buffer1.Print();//g
//     Buffer buffer2;
//     buffer2.WriteAsBuffer(buffer1);
//     buffer1.Print();//'\n'
//     buffer2.Print();//'g\n'
//     buffer1.WriteAsString("hi\njl");
//     buffer1.Print();//'hi\njl'
//     std::string str1=buffer1.ReadAsLine();
//     std::cout<<str1<<std::endl;//'hi'
//     buffer1.Print();//'jl'
// }
// Channel CreateChannel(int fd,const char* s)
// {
//     using callback=std::function<void()>;
//     callback r=[s]() {std::cout<<s<<" readCallback"<<std::endl;};
//     callback w=[s](){std::cout<<s<<" writeCallback"<<std::endl;};
//     callback e=[s](){std::cout<<s<<" errorCallback"<<std::endl;};
//     callback h=[s](){std::cout<<s<<" hupCallback"<<std::endl;};
//     callback ev=[s](){std::cout<<s<<" eventCallback"<<std::endl;};
//     Channel ch(fd);
//     ch.SetReadCallback(r);
//     ch.SetWriteCallback(w);
//     ch.SetErrorCallback(e);
//     ch.SetCloseCallback(h);
//     ch.SetEventCallback(ev);
//     return ch;

// }
// void ChannleTestServer()
// {
//     int epollfd=epoll_create(1);
//     if(epollfd==-1){ERROR_LOG("EPOLL CREATE ERROR");}
//     //监听listen字节
//     Socket server;
//     server.Create();
//     server.Bind("0.0.0.0",8700);
//     Channel lch=CreateChannel(server.Fd(),"Listen");
//     struct epoll_event l_event,events[10];
//     l_event.data.ptr=&lch;
//     l_event.events=lch.Events();
//     if(epoll_ctl(epollfd,EPOLL_CTL_ADD,lch.Fd(),&l_event)==-1)
//     {
//         ERROR_LOG("EPOLL REGISTER ERROR");abort();
//     }
//     else{DEBUG_LOG("EPOLL RESGISTER SUCCESS");}
//     pthread_mutex_lock(&mutex);
//     server.Listen();
//     server_ready=1;
//     pthread_cond_signal(&cond);
//     pthread_mutex_unlock(&mutex);
//     while(1)
//     {
//         sleep(1);
//         int nfds=epoll_wait(epollfd,events,10,0);
//         //events 事件的数组地址，10大小， -1永久阻塞
//         if(nfds==-1) {ERROR_LOG("EPOLL WAIT ERROR");abort();}
//         if(nfds==0){DEBUG_LOG("NO EVENTS EPOLL");}
//         for(int i=0;i<nfds;i++)
//         {
//             Channel* ptr=(Channel*)events[i].data.ptr;
//             if(ptr->Fd()==lch.Fd())
//             {
//                 //获得连接
//                 ptr->SetRevents(events[i].events);
//                 ptr->HandleEvent();
//                 int accept_fd=server.Accept();
//                 Channel ach=CreateChannel(accept_fd,"accept");
//                 struct epoll_event a_event;
//                 a_event.data.ptr=&ach;
//                 a_event.events=ach.Events();
//                 if(epoll_ctl(epollfd,EPOLL_CTL_ADD,ach.Fd(),&a_event)==-1)
//                 {
//                     ERROR_LOG("EPOLL REGISTER ERROR");
//                 }
//             }
//             else{
//                 //数据传输fd
//                 ptr->SetRevents(events[i].events);
//                 ptr->HandleEvent();
//                 int fd=ptr->Fd();
//                 char ch[20];
//                 if(read(fd,ch,sizeof(ch))==-1)
//                 {
//                     ERROR_LOG("READ ERROR"); abort();
//                 }
//                 printf("Read Content: %s\n",ch);
//                 break;
//                 // if(write(fd,ch,sizeof(ch))==-1)
//                 // {
//                 //     //回显
//                 //     ERROR_LOG("WRITE ERROR");abort();
//                 // }
//             }
//         }

//     }
// }
// void forkTest()
// {
//     pid_t pid=fork();
//     if(pid==0)
//     {
//         DEBUG_LOG("%s:%d","Child Process",getpid());
//         SocketServer();
//     }
//     else{
//         if(pid==-1){ERROR_LOG("FORK ERROR");}
//         else{
//             DEBUG_LOG("%s:%d","Father Process",getpid());
//             SocketClient();
//         }
//     }
// }
// void PollerTestServer()
// {
//     Poller poller;
//     std::vector<Channel*> channels;
//     //监听listen字节
//     Socket server;
//     server.Create();
//     server.ReuseAddress();
//     server.ReusePort();
//     server.Bind("0.0.0.0",8700);
//     Channel lch=CreateChannel(server.Fd(),"Listen");
//     lch.EnableRead();
//     poller.AddEvent(&lch);
//     pthread_mutex_lock(&mutex);
//     server.Listen(); //保证在listen后再进行监听
//     server_ready=1;
//     pthread_cond_signal(&cond);
//     pthread_mutex_unlock(&mutex);
//     //注册释放连接任务
//     TimerWheel tw(&poller);
//     int afd=0;
//     //添加监听时钟时间
//     auto closeListenFd=[&](){DEBUG_LOG("No Activate Link! Close Server");};
//     TimerTask timertask(4,closeListenFd);
//     while(1)
//     {       
//         std::vector<Channel*> channels;
//         poller.Poll(channels);
//         for(int i=0;i<channels.size();i++)
//         {
            
//             Channel* ptr=static_cast<Channel*>(channels[i]);
//              if(ptr->Fd()==lch.Fd())
//             {
//             //获得连接
//                 int accept_fd=server.Accept();
//                 afd=accept_fd;
//                 Channel ach=CreateChannel(accept_fd,"read");
//                 ach.EnableRead();
//                 poller.AddEvent(&ach);
//                 tw.AddTask(timertask);// 添加释放连接任务
//             }
//             else if(ptr->Fd()==tw.TimerFd())
//             {
//                 DEBUG_LOG("OnTime Run");
//                 continue;
//             }
//             else if(afd==ptr->Fd()){
//                 //数据传输fd
//                 printf("%d\n",ptr->Fd());
//                 int fd=ptr->Fd();
//                 char ch[20]={0};
//                 //为什么read这个会触发
//                 int n=read(fd,ch,sizeof(ch));
//                 if(n==-1)
//                 {
//                     ERROR_LOG("READ ERROR"); abort();
//                 }
//                 else if(n==0)
//                 {
//                     tw.DelayTask(3,timertask);
//                 }
//                 else{
//                     printf("ACCEPT_FD:%d",ptr->Fd());
//                     printf("Read Content: %s\n",ch);
//                 }
//             }
//             else{
//                 printf("FD:%d",ptr->Fd());
//             }
//         }
//     }

// }
// void* ServerThread(void* arg){
//      std::cout<<"Server Thread PID: "<<pthread_self()<<std::endl;
//     // SocketServer();
//     // ChannleTestServer();
//      PollerTestServer();
//      pthread_exit(nullptr);
// }
// void* ClientThread(void* arg){
//     std::cout<<"Client Thread PID: "<<pthread_self()<<std::endl;
//     SocketClient();

//     pthread_exit(nullptr);
// }
// void pthreadTest(){
//     DEBUG_LOG("RUN SERVER CLIENT PTHREAD");
//     pthread_t cpid;
//     pthread_t spid;
//     if(0!=pthread_create(&spid,nullptr,ServerThread,nullptr))
//     {
//         DEBUG_LOG("SERVER PTHREAD CREATE ERROR");
//     }
//     if(0!=pthread_create(&cpid,nullptr,&ClientThread,nullptr))
//     {
//         DEBUG_LOG("CLIENT PTHREAD CREATE ERROR");
//     }
//     void* retval_s;
//     void* retval_c;
//     pthread_join(spid,&retval_s);
//     pthread_join(cpid,&retval_c);

// }
// void TimerWheelTest()
// {   
    
//     Poller poller;
//     TimerWheel tw(&poller);
//     // poller.Poll();
//     using Callback=std::function<void()>;
//     Callback cb0=[](){DEBUG_LOG("第0个时间任务执行,第3秒执行");};
//     TimerTask t1(3,cb0);
//     std::cout<<"Task0ID:"<<t1.TaskId()<<std::endl;
//     tw.AddTask(t1);//添加任务
//     TimerTask t2(5,[](){DEBUG_LOG("第1个时间任务执行，延迟后会在第10秒执行");});
//     std::cout<<"Task1ID:"<<t2.TaskId()<<std::endl;
//     tw.AddTask(t2);
//     tw.DelayTask(5,t2);
//     TimerTask t3(3,[](){DEBUG_LOG("第3个时间任务执行，不会执行");});
//     while(1)
//     {
//         tw.OnTime();
//     }
    
    
//     std::vector<Channel*> vch;
//     std::cout<<vch.size()<<std::endl;
    
//     poller.Poll(vch);
//     std::cout<<vch.size()<<std::endl;
// }
// void SocketClient()
// {
//     Socket client;
//     pthread_mutex_lock(&mutex);
//     while(server_ready==0)
//     {
//         pthread_cond_wait(&cond,&mutex);
//     }
//     pthread_mutex_unlock(&mutex);
//     int fd=client.CreateClient("127.0.0.1",8500);
//     char ch[]="Hello Server!";
//     for(int i=1;i<=10;i++)
//     {
//         sleep(1);
//         ssize_t ret=send(fd,ch,sizeof(ch),0);
//         if(ret==-1||ret==0){ERROR_LOG("Write Error");abort();}
//         printf("Write Content: %s\n",ch);
//     }
//     sleep(10);
// }
// void EventLoopTest()
// {
//     //主从线程验证EventLoop LoopThread LoopThreadPool
//     //1. 主线程负责监听连接: 创建EventLoop对象，注册监听事件
//     //2. listen_fd可读->触发回调->分配连接
//     //3. 线程池的子线程负责接收信息
//     //4. 结束子线程
//     // 创建线程池
//     EventLoop baseloop;
//     LoopThreadPool pool(&baseloop);
//     pool.SetThreadCount(1);
//     pool.Create();//三个线程运行
//     Socket sk;
//     sk.Create();
//     sk.ReuseAddress();
//     sk.ReusePort();
//     sk.Bind("0.0.0.0",8500);
//     Channel ch(&baseloop,sk.Fd());
//     auto listenCallback = [&]() {
//     int acceptfd = sk.Accept();
//     DEBUG_LOG("AcceptFD:%d", acceptfd);
//     EventLoop* nl = pool.NextLoop();
    
//     auto ach = std::make_shared<Channel>(nl, acceptfd);
    
//     auto acceptCallback = [acceptfd, ach]() {
//         DEBUG_LOG("ACCEPT CALL BACK");
//         char buf[20] = {0};
//         ssize_t ret = read(acceptfd, buf, sizeof(buf));
//         if (ret == -1) {
//             DEBUG_LOG("Read Error");
//         } else {
//             printf("%s\n", buf);
//         }
//     };
    
//     ach->SetReadCallback(acceptCallback);
//     ach->EnableRead();
// };

// ch.SetReadCallback(listenCallback);
//     ch.SetReadCallback(listenCallback);
//     ch.EnableRead();//自动上传到事件循环
//     pthread_mutex_lock(&mutex);
//     sk.Listen();
//     server_ready=1;
//     pthread_cond_signal(&cond);
//     pthread_mutex_unlock(&mutex);
//     baseloop.Start();

// }
// void* ServerThread(void* arg){
//      std::cout<<"Server Thread PID: "<<pthread_self()<<std::endl;
//     // SocketServer();
//     // ChannleTestServer();
//      EventLoopTest();
//      pthread_exit(nullptr);
// }
// void* ClientThread(void* arg){
//     std::cout<<"Client Thread PID: "<<pthread_self()<<std::endl;
//     SocketClient();

//     pthread_exit(nullptr);
// }
// void pthreadTest(){
//     DEBUG_LOG("RUN SERVER CLIENT PTHREAD");
//     pthread_t cpid1,cpid2,cpid3;
//     pthread_t spid;
//     if(0!=pthread_create(&spid,nullptr,&ServerThread,nullptr))
//     {
//         DEBUG_LOG("SERVER PTHREAD CREATE ERROR");
//     }
//     if(0!=pthread_create(&cpid1,nullptr,&ClientThread,nullptr))
//     {
//         DEBUG_LOG("CLIENT PTHREAD CREATE ERROR");
//     }
//     // if(0!=pthread_create(&cpid2,nullptr,&ClientThread,nullptr))
//     // {
//     //     DEBUG_LOG("CLIENT PTHREAD CREATE ERROR");
//     // }
//     // if(0!=pthread_create(&cpid3,nullptr,&ClientThread,nullptr))
//     // {
//     //     DEBUG_LOG("CLIENT PTHREAD CREATE ERROR");
//     // }
//     void* retval_s;
//     void* retval_c1,*retval_c2,*retval_c3;
//     pthread_join(spid,&retval_s);
//     pthread_join(cpid1,&retval_c1);
//     // pthread_join(cpid2,&retval_c2);
//     // pthread_join(cpid3,&retval_c3);

// }
// void Test(){
//     // AnyTest();
//     // BufferTest();
//     // forkTest();
    
//      pthreadTest();
//     // TimerWheelTest();
    
// }

//     std::mutex mtx;
//     std::condition_variable cv;
//     bool ready=false;
// void AcceptorTest(){
//     //主线程->Acceptor 子线程->Connection
//     //1.创建baseloop
//     EventLoop baseloop;
//     Acceptor acceptor(&baseloop,8500);
//     LoopThreadPool pool(&baseloop);
//     pool.SetThreadCount(1);
//     pool.Create();
//     int connid=1;
//     using MessageCallback=std::function<void(const PtrConnection&,Buffer*)> ;
//     using ConnectedCallback= std::function<void(const PtrConnection&)>;
//     using ClosedCallback=std::function<void(const PtrConnection&)>;
//     using AnyEventCallback=std::function<void(const PtrConnection&)>;
//     MessageCallback mcb=[](const PtrConnection& conn,Buffer* buff){
//         DEBUG_LOG("MESSAGE CALLBACK");
//         // char buf[10];


//         buff->Print();
//         char buf[20];
//         buff->ReadBufferAndPop(buf,sizeof(buf));
//         buff->Print();
//         printf("%s\n",buf);
//         conn->Send(buf,sizeof(buf));
//     };
//     ConnectedCallback ccb=[](const PtrConnection& conn){
//         DEBUG_LOG("Connected Callback");
//     };
//     ClosedCallback clcb=[](const PtrConnection& conn){
//         DEBUG_LOG("CLOSED CALL BACK");
//     };
//     AnyEventCallback acb=[](const PtrConnection& conn){
//         DEBUG_LOG("ANY EVENT CALLBACK");
//     };
//     std::unordered_map<int,PtrConnection> ump;
//     //延迟生命周期，避免conn对像在函数内部被释放
//     auto acceptCallback=[&](int fd){
//         //在子线程中调用
//         //获取线程池的线程
//         DEBUG_LOG("ACCEPT CALL BACK");
//         EventLoop* loop=pool.NextLoop();
//         //构建连接
//         assert(-1!=fcntl(fd,F_GETFD));
//         PtrConnection self=std::make_shared<Connection>(loop,connid,fd);
//         ump[connid]=self;
//         self->SetContent("hello");
//         self->SetConnectedCallback(ccb);
//         self->SetClosedCallback(clcb);
//         self->SetAnyEventCallback(acb);
//         self->SetMessageCallback(mcb);
//         self->Established();
//     };
//     acceptor.SetAcceptCallback(acceptCallback);
//     acceptor.Listen();

//     {
//         std::unique_lock<std::mutex> lock(mtx);
//         ready=true;
//     }
//     cv.notify_all();
//     baseloop.Start();
    
// }
// void ClientSocket()
// {
//     Socket socket;
//     {
//         std::unique_lock<std::mutex> lock(mtx);
//         cv.wait(lock,[]{return ready;});
//     }

//     int fd=socket.CreateClient();
//     char buff[]={"hello"};
//     socket.NonBlockingSend(buff,sizeof(buff));
//     char buff1[20]={0};
//     socket.Recv(buff1,sizeof(buff));
//     printf("%s\n",buff1);
//     DEBUG_LOG("Sleep 10");
//     sleep(10);
//     DEBUG_LOG("Sleep 10");

// }
// int main()
// {

//     std::thread t1(AcceptorTest);
//     std::thread t2(ClientSocket);
//     t1.join();
//     t2.join();
//     return 0;


// }
#include"../server.hpp"
class EchoServer {
    private:
        TcpServer _server;
    private:
        void OnConnected(const PtrConnection &conn) {
            DEBUG_LOG("NEW CONNECTION:%p", conn.get());
        }
        void OnClosed(const PtrConnection &conn) {
            DEBUG_LOG("CLOSE CONNECTION:%p", conn.get());
        }
        void OnMessage(const PtrConnection &conn, Buffer *buf) {
            DEBUG_LOG("CLOSE OnMessage:%p", conn.get());
            conn->Send(buf->ReadPosition(), buf->ReadAbleSize());
            buf->RMoveOffset(buf->ReadAbleSize());
        }
    public:
        EchoServer(int port):_server(port) {
            _server.SetThreadCount(3);
            _server.EnableInactiveRelease(8);
            _server.SetClosedCallback(std::bind(&EchoServer::OnClosed, this, std::placeholders::_1));
            _server.SetConnectedCallback(std::bind(&EchoServer::OnConnected, this, std::placeholders::_1));
            _server.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
        }
        void Start() { _server.Start(); }
};

int main()
{
    EchoServer server(8500);
    server.Start();
    return 0;
}