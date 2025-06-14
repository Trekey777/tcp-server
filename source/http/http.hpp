
#include"../server.hpp"
#include<sys/stat.h>
#include<fstream>
#include<regex>

//状态码
//文件后缀-文件类型

std::unordered_map<int,std::string> _statu_msg={
    {100,"Continue"},
    {101,"Switching Protocol"},
    {102,"Processing"},
    {103,"Early Hints"},
    {200,"OK"},
    {201,"Created"},
    {202,"Accepted"},
    {203,"Non-Authoritative Information"}
};
std::unordered_map<std::string,std::string> _mime_msg={
    {".aac","audio/aac"},
    {".abw","application/x-abiword"},
    {".txt","application/txt"}
};
//工具类

class Util{

    public:
    //字符串切割
    static size_t SeperateString(const std::string& str,const std::string& sep,std::vector<std::string>* array)
    {
        size_t offset=0;
        while(offset<str.size())
        {
            //'sep',pos
            size_t pos=str.find(sep,offset);
            if(pos==std::string::npos)
            {
                //not found
                array->push_back(str.substr(offset));
                return array->size();
            }
            else{
                //
                if(offset!=pos)
                {
                    array->push_back(str.substr(offset,pos-offset));
                }
                offset=pos+sep.size();
                std::cout<<offset<<std::endl;
            }
        }
        return array->size();
    }
    //读文件
    static bool ReadFile(const std::string& filename,std::string* str)
    {
        //打开文件
        std::ifstream ifs(filename.c_str(),std::ifstream::binary);
        //检验错误
        if(!ifs.is_open())
        {
            ERROR_LOG("Open File %s Error",filename.c_str());
            return false;
        }
        //获取文件字符数量
        //0表示相对seekg的偏移量
        ifs.seekg(0,ifs.end);
        size_t filesize=ifs.tellg();
        ifs.seekg(0,ifs.beg);
        str->resize(filesize);
         //读取文件
        ifs.read(&(*str)[0],filesize);
        //检验错误
        if(!ifs.good())
        {
            ERROR_LOG("Read File %s Error",filename.c_str());
            return false;
        }
        //关闭文件
        ifs.close();
        return true;
    }
    static bool WriteFile(const std::string& filename,const std::string& str)
    {
        std::ofstream ofs(filename.c_str(),std::ofstream::binary|std::ofstream::trunc);
        if(!ofs.is_open())
        {
            ERROR_LOG("Write File %s Error",filename.c_str());
            return false;
        }
        ofs.write(str.c_str(),str.size());
        if(!ofs.good())
        {
            ERROR_LOG("Write File %s Error",filename.c_str());
            return false;
        }
        ofs.close();
        return true;
    }
    static std::string StatusCode(int code)
    {
        auto it=_statu_msg.find(code);
        if(it!=_statu_msg.end())
        {
            return it->second;
        }
        return "Unknow";
    }
    static  std::string MimeCode(const std::string& filepath)
    {
        size_t pos=filepath.find_last_of(".");
        if(pos==std::string::npos)
        {
            ERROR_LOG("Invalid Filepath Name");
            return "application/octet-stream";
        }
        else{
            std::string s=filepath.substr(pos);
            auto it=_mime_msg.find(s);
            if(it!=_mime_msg.end())
            {
                return it->second;

            }
            ERROR_LOG("Not Found Mime %s",s.c_str());
            return "application/octet-stream";

        }
    }
    static bool IsDirectory(const std::string& filepath)
    {
        struct stat st;
        int result=stat(filepath.c_str(),&st);
        if(result==-1){ERROR_LOG("Get Filepath Stat: %s Error",filepath.c_str());return false;}
        return S_ISREG(st.st_mode);
    }
    static bool IsRegularFile(const std::string& filepath)
    {
        struct stat st;
        int ret=stat(filepath.c_str(),&st);
        if(ret==-1){ERROR_LOG("Get Filepath Stat: %s Error",filepath.c_str());return false;}
        return S_ISDIR(st.st_mode);
    }

    static std::string EncodeUrl(const std::string& rawstr,bool is_query=false)
    {
        //非ascii字符集->转换为有效的ASCII字符集
        //编码字符->%HH格式
        //非编码字符->数字/字母 . - _ ~
        //空格 转成 + 不需要按格式输出
        std::string url;
        //解决utf-8r字符需要用到unsigned cha
        for(unsigned char s:rawstr)
        {
            if(isalnum(s)||s=='.'||s=='-'||s=='_'||s=='~')
            {
                url+=s;
                continue;
            }
            if(s==' '&&is_query==true)
            {
                url+='+';
                continue;
            }
            char tmp[4];
            snprintf(tmp,4,"%%%02X",s);
            url+=tmp;
        }
        return url;
    }
    static char HXtoI(char s)
    {
        if(s>='a'&&s<='z')
        {
            return s-'a'+10;
        }
        else if(s>='A'&&s<='Z')
        {
            return s-'A'+10;
        }
        else if(s>='0'&&s<='9')
        {
            return s-'0';
        }
        else{
            ERROR_LOG("Covert char %c from Url from HX to I Error",s);
            return 0;
        }

    }
    static std::string DecodeUrl(const std::string& url,bool is_query=false)
    {
        std::string str;
        for(int i=0;i<url.size();i++)
        {
            char s=url[i];
            if(isalnum(s)||s=='.'||s=='_'||s=='-'||s=='~')
            {
                str+=s;
                continue;
            }
            if(s=='+'&&is_query==true)
            {
                str+=' ';
                continue;
            }
            if(s=='%')
            {
                char r=HXtoI(url[i+1])<<4|HXtoI(url[i+2]);
                str+=r;
                i+=2;
            }
            else{
                ERROR_LOG("Invalid Char %c in Url",s);     
            }
        }
        return str;
    }
    static bool ValidePath(const std::string& path)
    {
        std::cout<<path<<std::endl;
        //相对根目录 /index/a/../..
        int level=0;
        std::vector<std::string> array;
        size_t size=SeperateString(path,"/",&array);
        for(int i=0;i<size;i++)
        {
            if(i==0&&array[i]==".")
            {
                continue;
            }
            if(array[i]=="..")
            {
                level--;
                if(level<0)
                {
                    return false;
                }
            }
            else{
                level++;
            }
        }
        return true;
    }
};
//http请求解析
class HttpRequest{
    public:
    // 方法 资源路径（路径+查询参数） HTTP版本号
    // 报头字段
    // 报文
    std::string _method;
    std::string _path;
    std::string _version;
    std::string _body;
    std::smatch _matches;//正则表达式结果
    std::unordered_map<std::string,std::string> _headers;
    std::unordered_map<std::string,std::string> _params;
    public:
    HttpRequest():_version("HTTP/1.1"){}
    void Reset()
    {
        _method.clear();
        _path.clear();
        _version="HTTP/1.1";
        std::smatch matches;
        _matches.swap(matches);
        _body.clear();
        _headers.clear();
        _params.clear();
    }
    //获取报头字段
    //检测报头字段是否存在
    //插入报头字段
    bool HasHeader(const std::string& key)
    {
        auto it=_headers.find(key);
        if(it!=_headers.end())
        {
            return true;
        }
        return false;
    }
    std::string GetHeader(const std::string & key)
    {
        bool ret=HasHeader(key);
        if(!ret)return "";
        return _headers.find(key)->second;        
    }
    bool InsertHeader(const std::string& key,const std::string& value)
    {
        bool ret=HasHeader(key);
        if(!ret)
        {
            _headers.insert({key,value});
            return true;
        }
        return false;
    }
    //插入查询参数
    //检测查询参数
    //插入查询参数
    bool HasParam(const std::string& key)
    {
        auto it =_params.find(key);
        if(it!=_params.end())
        {
            return true;
        }
        return false;
    }
    std::string GetParam(const std::string& key)
    {
        bool ret=HasParam(key);
        if(!ret){return "";}
        return _params[key];
    }
    bool InsertParam(const std::string& key,const std::string& value)
    {
        bool ret=HasParam(key);
        if(!ret){
            _params.insert({key,value});
            return true;
        }
        return false;
    }
    long GetContentLength()
    {
        std::string s=GetHeader("Content-Length");
        return stol(s);
    }
    
    //获取正文长度
    //判断是否短连接
    bool IsLongConnection()
    {
        std::string s=GetHeader("Connection");
        if(s=="keep-alive"){
            return true;
        }
        return false;
    }
};
//HTTP接收状态枚举
// HTTP响应类
//组装响应报文返回给客户端
// 版本 状态码 状态码的意思\r\n
// 报头字段
// 重定向url
// 报文内容
class HttpResponse{
    public:
    std::string _version;
    int _status;
    bool _is_redirect;
    std::string _redirect_url;
    std::unordered_map<std::string,std::string> _headers;
    std::string _body;
    public:
    HttpResponse():_is_redirect(false),_status(201),_version("HTTP/1.1"){}
    void Reset()
    {
        _version="HTTP/1.1";
        _status=201;
        _is_redirect=false;
        _redirect_url.clear();
        _headers.clear();
        _body.clear();
    }
    bool InsertHeader(const std::string& key,const std::string& value)
    {
        _headers.insert({key,value});
        return true;
    }
    bool HasHeader(const std::string& key)
    {
        auto it=_headers.find(key);
        if(it==_headers.end())
        {
            return false;
        }
        return true;
    }
    std::string GetHeader(const std::string& key)
    {
        auto it=_headers.find(key);
        if(it!=_headers.end())
        {
            return it->second;
        }
        return "";
    }
    void SetContent(const std::string& content,const std::string& type="text/html")
    {
        _body=content;
        InsertHeader("Content-Type",type);
    }
    void SetRedirct(const std::string& url, int status=302)
    {
        _status=status;
        _redirect_url=url;
    }
    bool IsLongConnection()
    {
        if(GetHeader("Connection")=="keep-alive")
        {
            std::cout<<GetHeader("Connection")<<std::endl;
            return true;
        }
        return false;
    }

};
// HTTP报文解析状态响应状态
//解析请求行 头 报文 结束
typedef enum{
    HTTP_RECV_ERROR,
    HTTP_RECV_LINE,
    HTTP_RECV_HEADER,
    HTTP_RECV_BODY,
    HTTP_RECV_OVER
}HttpRecvStatu;
#define MAXLINE 1024
//报文解析类
class HttpContent{
public:
    HttpRecvStatu _recv_statu;
    int _statu_code;
    HttpRequest _request;

    // 1.解析请求行ParseHttpLine-> 方法 资源路径查询字符串 版本\r\n
    bool ParseHttpLine( std::string& httpline)
    {

        std::smatch matches;
        std::regex pattern("(GET|POST|PUT|DELETE) ([^\\s?]+)(?:\\?([^\\s]))? (HTTP/\\d\\.\\d)\r?\n");
        bool ret=std::regex_match(httpline,matches,pattern);
        if(ret==false)
        {
            ERROR_LOG("Regex match Error");
            return false;
        }
        _request._method=matches[1];
        _request._path=matches[2];
        // a=3&b=2+3;
        std::vector<std::string> array;
        Util::SeperateString(matches[3],"&",&array);
        for(auto&e :array)
        {
            std::vector<std::string> param_array;
            Util::SeperateString(e,"=",&param_array);
            std::string key=Util::DecodeUrl(param_array[0],true);
            std::string value=Util::DecodeUrl(param_array[1],true);
            _request.InsertParam(key,value);
        }
        _request._version=matches[4];
        return true;
    }
    bool RecvHttpLine( Buffer& buffer)
    {
        
        // 2.接收请求行-> 获取一行数据->判断太长，太短->http报头行解析->更新状态
        if(_recv_statu!=HTTP_RECV_LINE)return false;
        DEBUG_LOG("RecvHttpLine");
        std::string httpline=buffer.ReadAsLine();
        if(httpline==""){
            //判断buffer内部大小
            if(buffer.ReadAbleSize()>MAXLINE)
            {
                _recv_statu=HTTP_RECV_ERROR;
                _statu_code=414;//URL too long
                return false;
            }
            else{
                return true;
            }
        }
        //对httpline判断
        if(httpline.size()>MAXLINE)
        {
            _recv_statu=HTTP_RECV_ERROR;
            _statu_code=414;
            return false;
        }
        bool ret=ParseHttpLine(httpline);
        if(ret==true){
            _recv_statu=HTTP_RECV_HEADER;
            return true;
        }
        else
        {
            _statu_code=400;//Bad Request
            _recv_statu=HTTP_RECV_ERROR;       
            return false;
        }
        
    }
    bool RecvHttpHeader(Buffer& buff)
    {
        // 3. 接收头处理
        
        if(_recv_statu!=HTTP_RECV_HEADER)return false;
        DEBUG_LOG("RecvHttpHeader");
        while(1)
        {
            std::string httpheader=buff.ReadAsLine();
            if(httpheader=="")
            {
                if(buff.ReadAbleSize()>MAXLINE)
                {
                    _statu_code=400;
                    _recv_statu=HTTP_RECV_ERROR;
                    return false;
                } 
                return true;   
            }
            if(httpheader.size()>MAXLINE)
            {
                _statu_code=400;
                _recv_statu=HTTP_RECV_ERROR;
                return false;
            }
            bool ret=ParseHttpHeader(httpheader);
            if(!ret)
            {
                _statu_code=400;
                _recv_statu=HTTP_RECV_ERROR;
                return false;
            }
            if(httpheader=="\r\n"||httpheader=="\n");
            {
                break;
            }
        }
        _statu_code=200;
        _recv_statu=HTTP_RECV_BODY;
        return true;

    }
    bool ParseHttpHeader(std::string& httpheader)
    {
        // 4. 解析报头处理
        std::string header=httpheader;
        if(httpheader.back()=='\n'){header.pop_back();}
        if(httpheader.back()=='\r'){header.pop_back();}
        std::vector<std::string> array;
        Util::SeperateString(header,":",&array);
        if(array.size()==0){return false;}
        _request.InsertHeader(array[0],array[1]);
        return true;
    }
    bool RecvHttpBody( Buffer& buff)
    {
        // 5. 解析报文处理

        if(_recv_statu!=HTTP_RECV_BODY)return false;
        DEBUG_LOG("RecvHttpBody");
        size_t needsize=(size_t)_request.GetContentLength()-_request._body.size();
        if(needsize>buff.ReadAbleSize())
        {
            _request._body.append(buff.ReadAsString(buff.ReadAbleSize()));
            return true;
        }
        _request._body.append(buff.ReadAsString(needsize));
        _statu_code=200;
        _recv_statu=HTTP_RECV_OVER;
        return true;
    }
    // bool ParseHttpBody()
    // {
    //     // 6. 接收报文处理
    // }
    HttpContent():_recv_statu(HTTP_RECV_LINE),_statu_code(200){}
    void ReSet(){
        _statu_code=200;
        _recv_statu=HTTP_RECV_LINE;
        _request.Reset();
    }
    int StatuCode()
    {
        return _statu_code;
    }
    HttpRecvStatu RecvStatu()
    {
        return _recv_statu;
    }
    HttpRequest& RecvRequest()
    {
        return _request;
    }
    void RecvHttp(Buffer& buff)
    {
        switch(_recv_statu){
        case HTTP_RECV_LINE:RecvHttpLine(buff);
        case HTTP_RECV_HEADER:RecvHttpHeader(buff);
        case HTTP_RECV_BODY:RecvHttpBody(buff);
        }
        return;
    }
};
class HttpServer{
    // 私有变量
    
    //1. GET POST DELETE POST 的方法的处理函数数组 接收Request 返回Response
    //2. 静态资源根目录
    //3. TcpServer

    //4. 错误处理 ->构建错误页面
    //5. 构建响应代码 response->响应报文->缓冲区发送
    //6. 是否请求文件 相对根目录存在->方法限定->路径合法-> 路径变换->路径合并/自动补充->文件类型
    //7. 静态资源请求处理 读取文件内容
    //8. 功能型请求的分发处理 req rsp handlers 比如数据库之类的
    //9. 区分静态资源请求和功能性请求
    //10. 设置解析上下文
    //11. 缓冲区数据解析+处理 获取上下文  根据状态码判断 路由处理 重置上下文 短连接关闭
    //12. 路由表添加方法
};
