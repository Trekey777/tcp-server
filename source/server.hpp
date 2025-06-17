#ifndef __M_SERVER_H__
#define __M_SERVER_H__
#include<iostream>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<time.h>
#include<string>
#include<cstring>
#include<netinet/ip.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<typeinfo>
#include<type_traits>
#include<vector>
#include<sys/epoll.h>
#include<functional>
#include<stdlib.h>
#include<thread>
#include<signal.h>
#include<sys/timerfd.h>
#include<memory>
#include<unordered_map>
#include<sys/eventfd.h>
#include <mutex>
#include<condition_variable>
#include <thread>
#include<fcntl.h>
#include<cassert>
/*日志纪录*/
std::string getTime();
#define DEBUG 0
#define INFO 1
#define ERROR 2
#define LOG(level,format,...) do{ \
    if(errno!=0)\
    {printf("[%s][%s %s %d] "#format": %s\n",level,getTime().c_str(),__FILE__,__LINE__,##__VA_ARGS__,strerror(errno));}\
    else{printf("[%s][%s %s %d] "#format"\n",level,getTime().c_str(),__FILE__,__LINE__,##__VA_ARGS__);}\
}while(0) 
#define DEBUG_LOG(format,...) LOG("DEBUG",format,##__VA_ARGS__)
#define INFO_LOG(format,...) LOG("INFO",format,##__VA_ARGS__)
#define ERROR_LOG(format,...) LOG("ERROR",format,##__VA_ARGS__)
std::string getTime()
{
    time_t t=time(nullptr);
    struct tm* ptm=localtime(&t);
    char ct[20];
    strftime(ct,sizeof(ct),"%Y-%m-%d %T:%M:%S",ptm);
    return std::string(ct);
}
/*日志*/
#define LISTENBACKLOG 1024
class Socket{
private:
    int _sockfd;
public:
    //创建套接字
    void Create(){
        _sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(_sockfd==-1)
        {
            ERROR_LOG("Socket Create Error");
            abort();
        }
        DEBUG_LOG("Socket Create Success");
    }
    void Bind(const char* addr,uint16_t port)
    {
        struct sockaddr_in addr_in;
        addr_in.sin_family=AF_INET;
        addr_in.sin_port=htons(port);
        addr_in.sin_addr.s_addr=inet_addr(addr);
        int ret=bind(_sockfd,(const struct sockaddr*)&addr_in,sizeof(addr_in));
        if(ret==-1)
        {
            ERROR_LOG("Bind Address Error");
            abort();
        }
        DEBUG_LOG("Bind Address Success");
    }
    void Listen(int backlog=LISTENBACKLOG)
    {
        if(listen(_sockfd,backlog)==-1) {ERROR_LOG("Listen Error");abort();}
        DEBUG_LOG("Listen Success");
    }
    int Accept()
    {
        struct sockaddr_in addr_in;
        memset(&addr_in,0,sizeof(addr_in));
        socklen_t len=sizeof(addr_in);//没有初始化导致
        int accept_fd=accept(_sockfd,(struct sockaddr*)&addr_in,&len);
        if(accept_fd==-1){ERROR_LOG("Accept Error");abort();}
        char client_ip[INET_ADDRSTRLEN];
        //线程安全，自己提供缓冲区
        inet_ntop(AF_INET, &addr_in.sin_addr, client_ip, INET_ADDRSTRLEN);
        DEBUG_LOG("Accept Success Client IP: %s Port: %d", client_ip,ntohs(addr_in.sin_port));
        return accept_fd;
    }
    void Connect(const char* addr,uint16_t port)
    {
        struct sockaddr_in addr_in;
        addr_in.sin_family=AF_INET;
        addr_in.sin_port=htons(port);
        addr_in.sin_addr.s_addr=inet_addr(addr);
        if(connect(_sockfd,(const struct sockaddr*)&addr_in,sizeof(addr_in))==-1)
        {
            ERROR_LOG("Connect Error");
            abort();
        }
        DEBUG_LOG("Connect Success");
    }
    void Close()
    {
        if(_sockfd!=-1)
        {
            close(_sockfd);
            _sockfd=-1;
        }

        
    }
    void ReuseAddress(){
        int opt=1;
        if(setsockopt(_sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))==-1)
        {
            ERROR_LOG("SET SOCK RESUEADDR ERROR");abort();
        }
        DEBUG_LOG("SET SOCK REUSEADDR SUCCESS");
    }
    void ReusePort(){
        int opt=1;
        if(setsockopt(_sockfd,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt))==-1)
        {
            ERROR_LOG("SET SOCK REUSEPORT ERROR");abort();
        }
        DEBUG_LOG("SET SOCK REUSEPORT SUCCESS");
    }
    void ReusePortandAddress(){
        ReuseAddress();
        ReusePort();
    }
    void NonBlock(){
        int flags=fcntl(_sockfd,F_GETFL,0);
        if(flags==-1){ERROR_LOG("GET FD FLAGS ERROR");abort();}
        if (!(flags&O_NONBLOCK)){
            if(fcntl(_sockfd,F_SETFL,flags|O_NONBLOCK)!=0)
            {
                ERROR_LOG("SET FD FLAGS ERROR");abort();
            }
        }
    }

public:
    Socket(){}
    ~Socket(){DEBUG_LOG("SOCKET DISCONSTRUCT");}
    Socket(int fd){_sockfd=fd;}
    int Fd(){return _sockfd;}
    int CreateServer(uint16_t port,const char* addr="0.0.0.0",bool block=false)
    {
        Create();
        if(!block)NonBlock();
        ReusePortandAddress();
        Bind(addr,port);
        Listen();
        return _sockfd;
    }
    int CreateClient(const char*addr="127.0.0.1",uint16_t port=8500)
    {
        Create();
        Connect(addr,port);
        return Fd();
    }
    ssize_t Recv(void* buff,size_t len,int flag=0)
    {
        ssize_t ret=recv(_sockfd,buff,len,flag);
        if(ret<=0){
            //EAGAIN  非阻塞情况无数据可读 ret=-1
            //EINTR 阻塞情况下 被信号打断 ret=-1
            //SUCCESS 对端关闭 接收到0个字节 ret=0
            if(errno==EAGAIN||errno==EINTR){
                return 0; //需要重试
            }
            return -1;//对端关闭以及其他错误->关闭连接
        }
        return ret;
    }
    ssize_t Send(const void* buff,size_t len,int flag=0)
    {
        ssize_t ret=send(_sockfd,buff,len,flag);
        if(ret<=0)
        {
            //EAGAIN: 非阻塞状态下缓冲区无数据
            //EINTR: 阻塞状态下被信号打断
            if(errno==EAGAIN||errno==EINTR)
            {
                return 0;//重试
            }
            ERROR_LOG("Socket Send Error");
            return -1; //关闭连接
        }
        return ret;
    }
    ssize_t NonBlockingRecv(void* buff,size_t len)
    {
        return Recv(buff,len,MSG_DONTWAIT);
       
    }
    ssize_t NonBlockingSend(const void* buff,size_t len)
    {
        return Send(buff,len,MSG_DONTWAIT);
    }

};

class Any{
private:
    class Base{
    public:
        Base(){}
        virtual ~Base()=default;//虚函数析构避免泄漏
        virtual Base* clone() const =0;//纯虚函数
        virtual const std::type_info& type()const=0;
    };
    //模板类继承非模板类
    template<class T>
    class Derived: public Base{
        friend class Any;
    private:
        T _val;
    public:
        Derived(const T& val):_val(val){
        }
        // Derived(T&& val):_val(std::move(val)){}//右值引用冲突
        Base* clone() const override{
            return new Derived<T>(_val);
        }
        //typedid(T)返回一个const type_info对象，拷贝构造函数私有，绕开返回值的拷贝构造，直接用const 引用接收右值
        const std::type_info& type(){return typeid(T);}
        ~Derived(){}
    };
    Base* _content=nullptr; //使用基类指针不用指定类型
public:
    //无参构造
    Any(){}
    // 传值构造->自动推导类型T，初始化_contenxt指针
    template<class T>
    Any(T content):_content(new Derived<T>(content)){}
    //拷贝构造： 传递的是Any无法知道类型，不能直接初始化指针->通过clone接口,让Any类内部克隆再返回
    Any (const Any& other):_content(other._content?other._content->clone():nullptr){}
    ~Any(){delete _content;_content=nullptr;}
    template<class T>
    Any& operator=(const T& content)
    {
        // std::cout<<"赋值重载符"<<std::endl;
        Any tmp(content);//调用拷贝构造或者传参构造any 或者非any类型使用
        std::swap(tmp._content,_content);
        return *this;
    }
    //获取值
    template<class T>
    T& any_cast()const
    {
        //要访问成员变量，必须换成派生类指针，判断返回值是否为空
        assert(typeid(T)==_content->type());
        Derived<T> ptr=dynamic_cast<Derived<T>*>(_content);
        if(ptr==nullptr)
        {
            ERROR_LOG("Any Cast ERROR");abort();
        }
        return ptr->_val;
    }



};
#define MAXBUFFERSIZE 65535
class Buffer{
private:
    std::vector<char> _buffer;
    uint16_t _start;//
    uint16_t _end;// char默认一个字节
public:
    char* Begin(){
        //1.首元素地址->基于vector底层空间是连续的
        return &(*(_buffer.begin()));
    }
    char* ReadPosition()
    {
        return Begin()+_start;//数据写入的地方
    }
    char* WritePosition()
    {
        return Begin()+_end;//空数据第一个位置
    }
    // 可读区域
    uint16_t ReadAbleSize(){

        return _end-_start;//[_start,end)
    }
    uint16_t FrontArea(){
        return _start; //[0,_start)
    }
    uint16_t BackArea(){ 
        return _buffer.size()-_end;// [_end,size)
    }
    //扩容
    void EnlargeSize(int n){
        if(n<=_buffer.size())
        {
            return;
        }
        if(n>MAXBUFFERSIZE)
        {
            DEBUG_LOG("EnlargeSize Reach Limit, Current Size: 65535");
            _buffer.resize(MAXBUFFERSIZE);
            return;  
        }
        _buffer.resize(n);
        DEBUG_LOG("ENLARGE SPACE SUCCESS");
        return;
    }
    void MoveData(){
        uint16_t len=ReadAbleSize();
        //区域重叠
        memmove(Begin(),ReadPosition(),len);
        _start=0;
        _end=len;
        DEBUG_LOG("Move Data");
    }
    void ReadBuffer(void* buf,size_t count){
        //count>ReadAbleSize 读取所有数据
        if(count>ReadAbleSize())
        {
           count=ReadAbleSize();
        }
        memmove(buf,ReadPosition(),count);
        return;
    }
    void WriteBuffer(const void *buf,size_t count){
        if(count<=BackArea())
        {
            memmove(WritePosition(),buf,count);
            return;
        }
        if(count>FrontArea()+BackArea())
        {
            EnlargeSize(count+_buffer.size());
        }
        if(count>FrontArea()+BackArea())
        {
            DEBUG_LOG("Not Enough Space");
            return;
        }
        else
        {
            MoveData();
            memmove(WritePosition(),buf,count);
            return;
        }
    }

    void WMoveOffset(uint16_t offset){_end=_end+offset;}
    void RMoveOffset(uint16_t offset){_start+=offset;}
    void Clear(){_start=0;_end=0;_buffer.clear();}
public:
    Buffer():_start(0),_end(0),_buffer(MAXBUFFERSIZE){}
    ~Buffer(){Clear();}
    void ReadBufferAndPop(void* buff,size_t count){
        ReadBuffer(buff,count);
        RMoveOffset(count);
    }
    void WriteBufferAndPush(const void *buff,size_t count){
        WriteBuffer(buff,count);
        WMoveOffset(count);
    }
    void WriteAsString(const std::string& str)
    {
        WriteBuffer(str.c_str(),str.size());
        WMoveOffset(str.size());
    }
    std::string ReadAsString(size_t len)
    {
        char ch[len+1];
        memset(ch,0,len);
        ReadBuffer(ch,len);
        RMoveOffset(len);
        ch[len]='\0';
        return std::string(ch);
    }
    void WriteAsBuffer(Buffer& b){
        int count=b.ReadAbleSize();
        b.ReadBufferAndPop(WritePosition(),count);
        WMoveOffset(count);
    }
    void Print()const
    {
        for(int i=_start;i<_end;i++)
        {
            printf("%c",_buffer[i]);
        }
        printf("\n");
    }
    std::string ReadAsLine()
    {
        int end=_start;
        int count=0;
        for(int i=_start;i<_end;i++)
        {
            if(_buffer[i]=='\n')
            {
                end=i;
                break;
            }
        }
        if(end!=_end)
        {
            count=end-_start+1;//包含\n
            return ReadAsString(count);
        }
        else{
            return "";
        }//end==_end
        
    }
};
// 对事件的封装 只涉及最简单的读写挂起错误监控
class Poller;
class EventLoop;
class Channel
{
private:
    uint32_t _events;//事件设置
    uint32_t _revents;//事件回调
    int _fd; //触发事件的文件描述符
    std::string _name;
    EventLoop* _loop;
    bool _removed=false;
    using Callback=std::function<void()>;
    Callback _read_callback;// EPOLLIN //EPOLLRDHUP(发送剩下的数据)
    Callback _write_callback;//EPOLLOUT 
    Callback _closed_callback;//EPOLLHUP
    Callback _error_callback;//EPOLLERR
    Callback _event_callback;//任意事件触发
public:
    static int count;
    Channel(EventLoop* loop,int fd,const char* name=""):_fd(fd),_events(0),_revents(0),_loop(loop),_name(name){}
    ~Channel(){Clear();}
    int Fd(){return _fd;}
    int Events(){return _events;}
    void SetRevents(int events){_revents=events;}
    void Clear(){_events=0;_revents=0;}
    const char* GetName(){return _name.c_str(); }
    bool IsRemoved() const { return _removed; }
    // 设置移除状态（在Remove()中调用）
    void SetRemoved() { _removed = true; }
    //设置回调
    void SetReadCallback(const Callback& cb)
    {
        _read_callback=cb;
    }
    void SetWriteCallback(const Callback& cb)
    {
        _write_callback=cb;
    }
    void SetCloseCallback(const Callback& cb)
    {
        _closed_callback=cb;
    }
    void SetErrorCallback(const Callback& cb)
    {
        _error_callback=cb;
    }
    void SetEventCallback(const Callback& cb){_event_callback=cb;}
    //设置事件监控
    //读写监控需要显式设置其他不需要
    //读监控
    
    //写监控
    void Update();
    void Remove();
    bool ReadAble(){return _events&EPOLLIN;}
    bool WriteAble(){return _events&EPOLLOUT;}
    void EnableRead(){_events|=EPOLLIN;Update();}
    void EnableWrite(){_events|=EPOLLOUT;Update();}
    void DisReadAble(){_events&=~EPOLLIN;Update();}
    void DisWriteAble(){_events&=~EPOLLOUT;Update();}
    void DiableAll(){_events=0;}

    void HandleEvent(){//顺序问题
        if(_revents&EPOLLERR)
        {
            DEBUG_LOG("%s ERRORCALLBACK",_name.c_str());
            _error_callback();
            return;
        }
        if(_revents&EPOLLHUP) //连接双方关闭
        {
            DEBUG_LOG("%s ClosedCALLBACK",_name.c_str());
            _closed_callback();
            return;
        }
        if(_revents&EPOLLIN)//对端关闭发送Fin包
        {
            if(_name!="TimerWheel"){
                DEBUG_LOG("%s ReadCALLBACK",_name.c_str());
            }
            
            _read_callback();
        }
        if(_revents&EPOLLOUT)
        {
            DEBUG_LOG("%s WRITECALLBACK",_name.c_str());
            _write_callback();
        }
        if(_event_callback){
            _event_callback();}
    }
};

//Poller模块编写
#define MAX_EPOLLEVENTS 10
class Poller{
private:
    int _epfd;
    struct epoll_event _evs[MAX_EPOLLEVENTS];
    std::unordered_map<int,Channel*> _channels;
    void Create(){
        _epfd=epoll_create(MAX_EPOLLEVENTS);
        if(_epfd==-1){ERROR_LOG("EPOLL CREATE ERROR");abort();}
        DEBUG_LOG("EPOLL CREATE SUCCESS");
    }
    void Update(int op,Channel * ptr)
    {
        struct epoll_event event;
        event.data.ptr=ptr;
        event.events=ptr->Events();
        const char* channel_name=ptr->GetName();
        if(epoll_ctl(_epfd,op,ptr->Fd(),&event)==-1)
        {
            if(op==EPOLL_CTL_ADD)
            {
                ERROR_LOG("%s EVENTS ADD ERROR",channel_name);
            }
            else if(op==EPOLL_CTL_MOD){
                ERROR_LOG("%s EVENTS UPDATE ERROR",channel_name);
            }
            else{
                ERROR_LOG("%s EVENTS DELETE ERROR",channel_name);
            }
            abort();
            
        }
        if(op==EPOLL_CTL_ADD)
        {
            DEBUG_LOG("%s EVENTS ADD Success",channel_name);
        }
        else if(EPOLL_CTL_MOD)
        {
            DEBUG_LOG("%s EVENTS UPDATE Success",channel_name);
        }
        else{
            DEBUG_LOG("%s EVENTS DELETE Success",channel_name);
        }
        
    }
    // 可能会出现重复的情况
    bool HasChannel(Channel* channel){
        auto it=_channels.find(channel->Fd());
        if(it==_channels.end())
        {
            return false;
        }
        return true;
    }
public:
    Poller(){Create();}
    void Poll(std::vector<Channel*>& channels)//线程的共享变量
    {
            //每次都用新的events数组，不然需要手动删除
            int nfds=epoll_wait(_epfd,_evs,MAX_EPOLLEVENTS,-1);
            //nfds==-1可能是因为被信号打断此时不应该结束
            if(nfds<0)
            {
                if(errno==EINTR)
                {
                    return;
                }
                ERROR_LOG("EPOLL WAIT ERROR");abort();
            }
            else
            {
               for(int i=0;i<nfds;i++)
               {
                Channel* ptr=static_cast<Channel*>(_evs[i].data.ptr);
                ptr->SetRevents(_evs[i].events);
                ptr->HandleEvent();
                channels.push_back(ptr);
               }
            }
    }
    void UpdateEvent(Channel* pch)
    {
        if(HasChannel(pch))
        {
            Update(EPOLL_CTL_MOD,pch);
        }
        else
        {
            Update(EPOLL_CTL_ADD,pch);
        _channels.insert({pch->Fd(),pch});
        }
    }
    void RemoveEvent(Channel* pch){
        if(HasChannel(pch))
        {

            Update(EPOLL_CTL_DEL,pch);
            auto it=_channels.find(pch->Fd());
            if(it!=_channels.end())
            {
                _channels.erase(it);
            }
        }
        else{
            ERROR_LOG("Delete Nonexist Event");
            }
    }
};
class TimerWheel;
class TimerTask{
    //计时任务应该有的属性
private:
    using TaskCallback=std::function<void()>;
    using ReleaseCallback=std::function<void(int)>;
    int _taskid;
    int _delay;//延时时间, 默认30s后执行
    bool _cancel;//任务取消

    TaskCallback _task;//任务的回调函数
    ReleaseCallback _release;
public: 
    TimerTask(int taskid,int delay,TaskCallback cb):_taskid(taskid),_delay(delay),_cancel(false),_task(cb){
        if(delay>=60){ERROR_LOG("DELAY <=60");abort();}
    }
    void CancelTask(){_cancel=true;}
    int Delay(){return _delay;}
    void SetRelease(ReleaseCallback cb){_release=cb;}
    int TaskId(){return _taskid;}
    ~TimerTask(){
        if(!_cancel){_task();}
        else{DEBUG_LOG("CANCEL TASK SUCEESS");}
        _release(_taskid);}
};

class TimerWheel{
//定时器
private: 
    using TaskPtr=std::shared_ptr<TimerTask>;
    using WeakPtr=std::weak_ptr<TimerTask>;
    using TaskCallback=std::function<void()>;
    int _timerfd;
    int _step;//时间轮位置
    int _capacity;
    int second=0;
    EventLoop* _loop;
    std::vector<std::vector<TaskPtr>> _timerwheel;
    std::unique_ptr<Channel> _timer_channel;
    std::unordered_map<int,WeakPtr> _ump;//如果WeakPtr指向的对象释放，但是本身可能没有释放,需要在任务调用结束后手动释放
public:
    TimerWheel(EventLoop *loop):_timerfd(CreateTimer()),_step(0),_capacity(60),_timerwheel(_capacity),_loop(loop), _timer_channel(new Channel(_loop,_timerfd,"TimerWheel")){
        _timer_channel->SetReadCallback(std::bind(&TimerWheel::OnTime,this));
        _timer_channel->EnableRead();
    }
    int TimerFd(){
        while(_timerfd==-1);
        return _timerfd;
    }
    static int CreateTimer()
    {
        //timerfd_create
        int timerfd=timerfd_create(CLOCK_MONOTONIC,0);
        if(timerfd==-1)
        {
            ERROR_LOG("TIMER CREATE ERROR");
        }
        struct itimerspec new_value; //每隔一秒超时一次 _timerfd可读一次
        new_value.it_interval.tv_sec=1;
        new_value.it_interval.tv_nsec=0;
        new_value.it_value.tv_sec=1;
        new_value.it_value.tv_nsec=0;//settime开始计时了
        if(timerfd_settime(timerfd,0,(const struct itimerspec*)&new_value,NULL)==-1)
        {
            ERROR_LOG("TIMER SETTIME ERROR");abort();
        }
        return timerfd;
    }
    uint64_t ExpireTime()
    {
        uint64_t time=0;
        if(read(_timerfd,(void*)&time,sizeof(time))==-1)
        {ERROR_LOG("READ TIMER ERROR");abort();}
        return time;
    }
    void OnTime(){
        uint64_t time=ExpireTime();
        second+=time;
        // DEBUG_LOG("第%d秒",second);
        for(int i=0;i<time;i++)
        {
            _timerwheel[_step].clear();
            _step=(_step+1)%_capacity;
        }
    }
    bool HasTimer(int taskid)
    {
        auto it=_ump.find(taskid);
        return it!=_ump.end();
    }
    void AddTask(int taskid,int delay,TaskCallback cb)
    {
        //无论是否是引用 make_shared都是在堆上开辟空间拷贝构造t
        TaskPtr ptr=std::make_shared<TimerTask>(taskid,delay,cb);
        ptr->SetRelease([this](int x){this->ClearWeakPtr(x);});
        int pos=(_step+delay)%_capacity;
        _timerwheel[pos].push_back(ptr);
        std::weak_ptr<TimerTask> wptr=ptr;
        _ump[taskid]=wptr;
    }
    void CancelTask(int taskid)
    {
        std::weak_ptr<TimerTask> wptr=_ump[taskid];
        if(!wptr.expired())
        {
            std::shared_ptr<TimerTask> ptr=wptr.lock();
            if(ptr)
            {
                ptr->CancelTask();
            }
        }
    }
    void DelayTask(int taskid)
    {
        std::weak_ptr<TimerTask> wptr=_ump[taskid];
        if(!wptr.expired()){
            std::shared_ptr<TimerTask> ptr=wptr.lock();
            if(ptr){
                int pos=(_step+ptr->Delay())%_capacity;
                _timerwheel[pos].push_back(ptr);
            }
        }
    }
    void ClearWeakPtr(int taskid){
        auto it=_ump.find(taskid);
        if(it!=_ump.end())
        {
            _ump.erase(it);
        }   
    }
    ~TimerWheel(){
        _ump.clear();
        _timerwheel.clear();
    }
};
class EventLoop{
    private:
        using Functor=std::function<void()>;
        std::thread::id _thread_id;//线程ID
        int _event_fd; 
        Poller _poller;
        std::unique_ptr<Channel> _event_channel;
        std::vector<Functor> _task;
        std::mutex _mutex;
        TimerWheel _timer_wheel;
    public:
    //执行任务池任务：1. 加锁获取数据 2. 遍历执行任务
    void RunTask()
    {
        std::vector<Functor> task;
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            swap(task,_task);
        }
        for(auto& e:task)
        {
            e();
        }
    }
    //创建 eventfd 
    static int CreateEventFd()
    {
        int eventFd=eventfd(0,0);//flags为0，初始化为0
        if(eventFd==-1){ERROR_LOG("Create EventFd Error");}
        return eventFd;
    }
    void ReadEventFd()
    {
        uint64_t ret;
        if(read(_event_fd,&ret,sizeof(ret))==-1)
        {
            ERROR_LOG("ERROR Read EventFd");
        }
    }
    void WeakEventFd()
    {
        uint64_t event=1;
        if(write(_event_fd,&event,sizeof(event))==-1)
        {
            ERROR_LOG("ERROR Write EventFd");
        }

    }
    //初始化，设定eventfd的回调函数
    EventLoop():_thread_id(std::this_thread::get_id()),
                _event_fd(CreateEventFd()),
                _event_channel(new Channel(this,_event_fd,"EventLoop")),
                _timer_wheel(this){
                _event_channel->SetReadCallback(std::bind(&EventLoop::ReadEventFd,this));
                _event_channel->EnableRead();
    }
    void Start()
    {
        while(1){
            
            std::vector<Channel*> channels;
            _poller.Poll(channels);
            RunTask();
        } 

    }
    bool IsInLoop()
    {
        if(_thread_id==std::this_thread::get_id())
        {
            return true;
        }
        return false;
    }
    void AssertInLoop()
    {
        assert(_thread_id==std::this_thread::get_id());
    }
    void RunInLoop(Functor cb)
    {
        if(IsInLoop()){
            //很重要 用于主线程Start()前执行
            return cb();

        }
       
        return QueueInLoop(cb);
    }
    void QueueInLoop(Functor cb)
    {
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            _task.push_back(cb);
        }
        WeakEventFd();
        
    }
    void UpdateEvent(Channel* ch)
    {
        RunInLoop(std::bind(&Poller::UpdateEvent,&_poller,ch));
        //std::placeholders::_1;
    }
    void RemoveEvent(Channel* ch)
    {
        RunInLoop(std::bind(&Poller::RemoveEvent,&_poller,ch));
    }
    //添加修改事件描述符
    //移除事件描述符
    // 不直接进入事件循环
    void AddTimerTask(int taskid,int delay,Functor cb)
    {
        _timer_wheel.AddTask(taskid,delay,cb);
    }
    void DelayTimerTask(int taskid)
    {
        _timer_wheel.DelayTask(taskid);
    }
    void CancelTimerTask(int taskid)
    {
        _timer_wheel.CancelTask(taskid);
    }
    bool HasTimer(int taskid)
    {
       return _timer_wheel.HasTimer(taskid);
    }
    //任务添加/刷新/取消到时间轮
    //判断时钟
};

void Channel::Remove()
{
    SetRemoved();
    DEBUG_LOG("Remove Channel: fd=%d, ptr=%p", _fd, this);
    return _loop->RemoveEvent(this);
}
void Channel::Update(){return _loop->UpdateEvent(this);}

class LoopThread{
    private:
        std::mutex _mutex; //锁
        std::condition_variable _cond; //条件变量
        EventLoop* _loop; //指针变量
        std::thread _thread; //EventLoop对应的线程
    private:
     void ThreadEntry()
     {
        EventLoop loop;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _loop=&loop;
            _cond.notify_all();
        }
        loop.Start();
     }
    public:
    LoopThread():_loop(NULL),_thread(std::thread(&LoopThread::ThreadEntry,this)){}
    EventLoop* GetLoop()
    {
        EventLoop* loop;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond.wait(lock,[&](){return _loop!=NULL;});
            loop=_loop;
        }
        return loop;
    }
    
};
class LoopThreadPool{
    private:
        int _thread_count;
        int _next_index;
        EventLoop* _baseloop;
        std::vector<EventLoop*> _loops;
        std::vector<LoopThread*> _threads;
    public:
        LoopThreadPool(EventLoop* baseloop):_thread_count(0),_next_index(0),_baseloop(baseloop){}
        void SetThreadCount(int count){_thread_count=count;}
        void Create()
        {
            if(_thread_count>0){
                _threads.resize(_thread_count);
                _loops.resize(_thread_count);
                for(int i=0;i<_thread_count;i++)
                {
                    _threads[i]=new LoopThread();
                    _loops[i]=_threads[i]->GetLoop();
                }
            }
        }
        EventLoop* NextLoop(){
            if(_thread_count==0)//
            {
                return _baseloop;
            }
            _next_index=(_next_index+1)%_thread_count; //Error floating point exception:_thread_count=0
            return _loops[_next_index];
        }
};
class Connection;
//连接状态
typedef enum{
    DISCONNECTED,DISCONNECTING,CONNECTING,CONNECTED,
}ConStatu;
using PtrConnection=std::shared_ptr<Connection>;
class Connection: public std::enable_shared_from_this<Connection>{
private:
    //循环 读/写缓冲区 
    EventLoop *_loop;
    int _connid;
    int _sockfd;
    Channel _channel;
    Any _context;
    ConStatu _statu;
    Socket _socket;
    Buffer _in_buffer;
    Buffer _out_buffer;
    bool _enable_inactive_release;
    using MessageCallback=std::function<void(const PtrConnection&,Buffer*)> ;
    using ConnectedCallback= std::function<void(const PtrConnection&)>;
    using ClosedCallback=std::function<void(const PtrConnection&)>;
    using AnyEventCallback=std::function<void(const PtrConnection&)>;
    //组件设置的
    MessageCallback _message_callback;
    ConnectedCallback _connected_callback;
    ClosedCallback _closed_callback;
    AnyEventCallback _any_event_callback;
    // 自动运行的
    ClosedCallback _server_closed_callback;
    private:
        void HandleRead(){
            //1.读取数据 2.保存到buffer内部 3. 调用连接回调
            DEBUG_LOG("Handle Read");
            char buff[65535];
            ssize_t ret=_socket.NonBlockingRecv(buff,sizeof(buff));
            if(ret<0)
            {
                DEBUG_LOG("HANDLE READ ERROR, READ %d bytes",static_cast<int>(ret));
                assert(fcntl(_sockfd,F_GETFD)!=-1);
                //出错了不关闭连接
                return ShutdownInLoop();
            }
            //ret==0 没有读取到数据
            _in_buffer.WriteBufferAndPush(buff,ret);//不是65535而是读取的大小
            if(_in_buffer.ReadAbleSize()>0)
            {
               return _message_callback(shared_from_this(),&_in_buffer);
            }
        }

        void HandleWrite(){
            DEBUG_LOG("Handle Write");
            //1.从_out_buffer读取数据-> 2.调用_socket.NonBlockingSend()发送数据
            ssize_t ret=_socket.NonBlockingSend(_out_buffer.ReadPosition(),_out_buffer.ReadAbleSize());
            if(ret<0)
            {
                //处理读缓冲区的数据 
                //? 什么场景下要需要处理可读数据
                if(_in_buffer.ReadAbleSize()>0)
                {
                    _message_callback(shared_from_this(),&_in_buffer);
                }
                return Release();
            }
            _out_buffer.RMoveOffset(ret);
            if(_out_buffer.ReadAbleSize()==0)
            {
                _channel.DisWriteAble();//关闭可写监控
                //调用write,send时可写状态
                if(_statu==DISCONNECTING)
                {
                    return Release();
                }
            }

        }
        void ShutdownInLoop()
        {
            DEBUG_LOG("ShutdownInLoop");
            //1. 改变连接状态状态 2.处理读缓冲区数据 3.处理写缓冲区数据，有数据->打开写监控(可能触发写handlewrite()), 没有数据->释放连接
            _statu=DISCONNECTED;
            if(_in_buffer.ReadAbleSize()>0)
            {
                if(_message_callback) _message_callback(shared_from_this(),&_in_buffer);
            }
            //有可发送数据打开写监控
            if(_out_buffer.ReadAbleSize()>0)
            {
                _channel.EnableWrite();

            }
            //没有可发送数据，关闭
            if(_out_buffer.ReadAbleSize()==0)
            {
                Release();
            }
        }
        static int count;
        void ReleaseInLoop()
        {
        //     if (_statu == DISCONNECTED) return;
            DEBUG_LOG("RELEASE IN LOOP: %d",++count);
            //1.修改连接状态 2.移除事件监控 3. 消除定时任务 4. 调用用户和系统的回调函数
            _statu=DISCONNECTED;
            _channel.Remove();
            _socket.Close();

            // if(_loop->HasTimer(_connid)){CancelInactiveReleaseInLoop();};//异步函数
            if(_closed_callback) _closed_callback(shared_from_this());
            if(_server_closed_callback) _server_closed_callback(shared_from_this());
        }
        void HandleClose()
        {
            DEBUG_LOG("HANDLE CLOSE");
            //处理可读区的数据
            if(_in_buffer.ReadAbleSize()>0)
            {
                _message_callback(shared_from_this(),&_in_buffer);
            }
            // return Release();

        }
        void HandleError()
        {
            DEBUG_LOG("HANDLE ERROR");
            return HandleClose();
        }
        void EstablishedInLoop(){
            //1.修改连接状态 2.启动事件监控 3.调用回调函数
            DEBUG_LOG("ESTABLISHEDINLOOP");
            assert(_statu==CONNECTING);
            _statu=CONNECTED;
            _channel.EnableRead();//触发可读事件
            if(_connected_callback) _connected_callback(shared_from_this());
        }
        void SendInLoop(Buffer &buff)
        {
            //1.把数据保存在写缓冲区中准备发送
            DEBUG_LOG("SendInLoop");
            if(_statu==DISCONNECTED) return;
             _out_buffer.WriteBufferAndPush(buff.ReadPosition(),buff.ReadAbleSize());
            if(_channel.WriteAble()==false){
                _channel.EnableWrite();//只要打开了可读状态，缓冲区不满的情况下基本就会一直触发hanlewrite
            }
        }
        void EnableInactiveReleaseInLoop(int sec)
        {
            //1.标志位置于true
            _enable_inactive_release=true;
            if(_loop->HasTimer(_connid))
            {
                _loop->DelayTimerTask(_connid);
            }
            //2.如果没有就添加
            _loop->AddTimerTask(_connid,sec,std::bind(&Connection::Release,this));
        }
        void CancelInactiveReleaseInLoop(){
            _enable_inactive_release=false;
            if(_loop->HasTimer(_connid))
            {
                _loop->CancelTimerTask(_connid);
            }
        }
        void UpgradeInLoop(const Any& context,
                            const ConnectedCallback &conn,
                            const MessageCallback &msg,
                            const ClosedCallback &closed,
                            const AnyEventCallback& event)
        {
            _context=context;
            _connected_callback=conn;
            _message_callback=msg;
            _closed_callback=closed;
            _any_event_callback=event;
        }
    public:
        Connection(EventLoop* loop,int connid,int sockfd):
            _connid(connid),
            _sockfd(sockfd),
            _enable_inactive_release(false),
            _loop(loop),
            _statu(CONNECTING),
            _socket(sockfd),
            _channel(loop,sockfd,"Connection"){
                _channel.SetReadCallback(std::bind(&Connection::HandleRead,this));
                _channel.SetWriteCallback(std::bind(&Connection::HandleWrite,this));
                _channel.SetCloseCallback(std::bind(&Connection::HandleClose,this));
                _channel.SetErrorCallback(std::bind(&Connection::HandleError,this));
            }
        ~Connection(){DEBUG_LOG("Release CONNECTED: %p",this);}
        int Fd(){return _sockfd;}
        int Id(){return _connid;}
        bool Connected(){return _statu==CONNECTED;}
        void SetContent(const Any& context){_context=context;}
        Any* GetContext(){return &_context;}
        void SetConnectedCallback(const ConnectedCallback& cb){_connected_callback=cb;}
        void SetMessageCallback(const MessageCallback& cb){_message_callback=cb;}
        void SetClosedCallback(const ClosedCallback& cb){_closed_callback=cb;}
        void SetAnyEventCallback(const AnyEventCallback& cb){_any_event_callback=cb;}
        void SetSrvClosedCallback(const ClosedCallback& cb){_server_closed_callback=cb;}
        void Established(){
            _loop->RunInLoop(std::bind(&Connection::EstablishedInLoop,this));
        }
        void Send(const char*data,size_t len){
            Buffer buf;
            buf.WriteBufferAndPush(data,len);
            //?move 按右值传递
            _loop->RunInLoop(std::bind(&Connection::SendInLoop,this,std::move(buf)));
        }
        void Shutdown(){
            _loop->RunInLoop(std::bind(&Connection::ShutdownInLoop,this));
        }
        void Release(){
            _loop->RunInLoop(std::bind(&Connection::ReleaseInLoop,this));
        }
        //启动非活跃释放连接
        void EnableInactiveRelease(int sec){
            _loop->RunInLoop(std::bind(&Connection::EnableInactiveRelease,this,sec));
        }
        //取消非活跃释放连接
        void CancelInactiveRelease(){
            _loop->RunInLoop(std::bind(&Connection::CancelInactiveReleaseInLoop,this));
        }
        //更新_context 以及回调函数
        void Upgrade(Any context,
            const MessageCallback& message_callback,
            const ClosedCallback& closed_callback,
            const AnyEventCallback& any_event_callback,
            const ConnectedCallback& connected_callback){
                _loop->AssertInLoop();
                _loop->RunInLoop(std::bind(&Connection::UpgradeInLoop,this,
                context,
                connected_callback,
                message_callback,
                closed_callback,
                any_event_callback
                ));
        }
};
int Connection::count=0;
//监听模块
class Acceptor{
private:
    Socket _socket;
    EventLoop* _loop;
    Channel _channel;
    using AcceptCallback=std::function<void(int)>;
    AcceptCallback _accept_callback;
private:
    void HandleRead(){
        int newfd=_socket.Accept();
        if(newfd<0)return;
        if(_accept_callback) _accept_callback(newfd);
    }
    int CreateServer(int port)
    {
        bool ret=_socket.CreateServer(port);
        assert(ret==true);
        return _socket.Fd();
    }
public:
    Acceptor(EventLoop* loop,int port):_socket(CreateServer(port)),_loop(loop),_channel(_loop,_socket.Fd(),"Acceptor")
    {
        _channel.SetReadCallback(std::bind(&Acceptor::HandleRead,this));
    }
    //保证先设定好SetAcceptCallback再进行监听
    void Listen(){_channel.EnableRead();}
    void SetAcceptCallback(const AcceptCallback &cb){_accept_callback=cb;}
};
class TcpServer{
    private:
        uint64_t _next_id;
        int _port;
        int _timeout;
        bool _enable_inactive_release;
        EventLoop _baseloop;
        Acceptor _acceptor;
        LoopThreadPool _pool;
        std::unordered_map<uint64_t,PtrConnection> _conns;
        using ConnectedCallback=std::function<void(const PtrConnection&)>;
        using MessageCallback=std::function<void(const PtrConnection&,Buffer *)>;
        using ClosedCallback=std::function<void(const PtrConnection&)>;
        using AnyEventCallback=std::function<void(const PtrConnection&)>;
        using Functor=std::function<void()>;
        ConnectedCallback _connected_callback;
        MessageCallback _message_callback;
        ClosedCallback _closed_callback;
        AnyEventCallback _event_callback;
    private:
        void RunAfterInLoop(const Functor &task,int delay){
            _next_id++;
            _baseloop.AddTimerTask(_next_id,delay,task);
        }
        void NewConnection(int fd)
        {
            _next_id++;
            PtrConnection conn(new Connection(_pool.NextLoop(),_next_id,fd));
            conn->SetMessageCallback(_message_callback);
            conn->SetClosedCallback(_closed_callback);
            conn->SetConnectedCallback(_connected_callback);
            conn->SetAnyEventCallback(_event_callback);
            conn->SetSrvClosedCallback(std::bind(&TcpServer::RemoveConnection,this,std::placeholders::_1));
            // if(_enable_inactive_release) conn->EnableInactiveRelease(_timeout);
            conn->Established();
            _conns.insert(std::make_pair(_next_id,conn));
        }
        void RemoveConnectionInLoop(const PtrConnection &conn)
        {
            int id=conn->Id();
            auto it=_conns.find(id);
            if(it!=_conns.end())
            {
                _conns.erase(it);
            }
        }
        void RemoveConnection(const PtrConnection &conn)
        {
            _baseloop.RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop,this,conn));
        }
    public:
        TcpServer(int port):
            _port(port),
            _next_id(0),
            _enable_inactive_release(false),
            _acceptor(&_baseloop,port),
            _pool(&_baseloop){
                _acceptor.SetAcceptCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
                _acceptor.Listen();//将监听套接字挂到baseloop上
            }
        void SetThreadCount(int count) { return _pool.SetThreadCount(count); }
        void SetConnectedCallback(const ConnectedCallback&cb) { _connected_callback = cb; }
        void SetMessageCallback(const MessageCallback&cb) { _message_callback = cb; }
        void SetClosedCallback(const ClosedCallback&cb) { _closed_callback = cb; }
        void SetAnyEventCallback(const AnyEventCallback&cb) { _event_callback = cb; }
        void EnableInactiveRelease(int timeout) { _timeout = timeout; _enable_inactive_release = true; }
        //用于添加一个定时任务
        void RunAfter(const Functor &task, int delay) {
            _baseloop.RunInLoop(std::bind(&TcpServer::RunAfterInLoop, this, task, delay));
        }
        void Start() { _pool.Create();  _baseloop.Start(); }
};
class NetWork {
    public:
        NetWork() {
            DEBUG_LOG("SIGPIPE INIT");
            signal(SIGPIPE, SIG_IGN);
        }
};
static NetWork nw;
#endif