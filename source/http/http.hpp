
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
    {203,"Non-Authoritative Information"},
    {204,"No Content"},
    {205,"Reset Content"},
    {206, "Partial Content"},
    {207,"Multi-Statu"},
    {208,  "Already Reported"},
    {226,  "IM Used"},
    {300,  "Multiple Choice"},
    {301,  "Moved Permanently"},
    {302,  "Found"},
    {303,  "See Other"},
    {304,  "Not Modified"},
    {305,  "Use Proxy"},
    {306,  "unused"},
    {307,  "Temporary Redirect"},
    {308,  "Permanent Redirect"},
    {400,  "Bad Request"},
    {401,  "Unauthorized"},
    {402,  "Payment Required"},
    {403,  "Forbidden"},
    {404,  "Not Found"},
    {405,  "Method Not Allowed"},
    {406,  "Not Acceptable"},
    {407,  "Proxy Authentication Required"},
    {408,  "Request Timeout"},
    {409,  "Conflict"},
    {410,  "Gone"},
    {411,  "Length Required"},
    {412,  "Precondition Failed"},
    {413,  "Payload Too Large"},
    {414,  "URI Too Long"},
    {415,  "Unsupported Media Type"},
    {416,  "Range Not Satisfiable"},
    {417,  "Expectation Failed"},
    {418,  "I'm a teapot"},
    {421,  "Misdirected Request"},
    {422,  "Unprocessable Entity"},
    {423,  "Locked"},
    {424,  "Failed Dependency"},
    {425,  "Too Early"},
    {426,  "Upgrade Required"},
    {428,  "Precondition Required"},
    {429,  "Too Many Requests"},
    {431,  "Request Header Fields Too Large"},
    {451,  "Unavailable For Legal Reasons"},
    {501,  "Not Implemented"},
    {502,  "Bad Gateway"},
    {503,  "Service Unavailable"},
    {504,  "Gateway Timeout"},
    {505,  "HTTP Version Not Supported"},
    {506,  "Variant Also Negotiates"},
    {507,  "Insufficient Storage"},
    {508,  "Loop Detected"},
    {510,  "Not Extended"},
    {511,  "Network Authentication Required"}
};
std::unordered_map<std::string, std::string> _mime_msg = {
    {".aac",        "audio/aac"},
    {".abw",        "application/x-abiword"},
    {".arc",        "application/x-freearc"},
    {".avi",        "video/x-msvideo"},
    {".azw",        "application/vnd.amazon.ebook"},
    {".bin",        "application/octet-stream"},
    {".bmp",        "image/bmp"},
    {".bz",         "application/x-bzip"},
    {".bz2",        "application/x-bzip2"},
    {".csh",        "application/x-csh"},
    {".css",        "text/css"},
    {".csv",        "text/csv"},
    {".doc",        "application/msword"},
    {".docx",       "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot",        "application/vnd.ms-fontobject"},
    {".epub",       "application/epub+zip"},
    {".gif",        "image/gif"},
    {".htm",        "text/html"},
    {".html",       "text/html"},
    {".ico",        "image/vnd.microsoft.icon"},
    {".ics",        "text/calendar"},
    {".jar",        "application/java-archive"},
    {".jpeg",       "image/jpeg"},
    {".jpg",        "image/jpeg"},
    {".js",         "text/javascript"},
    {".json",       "application/json"},
    {".jsonld",     "application/ld+json"},
    {".mid",        "audio/midi"},
    {".midi",       "audio/x-midi"},
    {".mjs",        "text/javascript"},
    {".mp3",        "audio/mpeg"},
    {".mpeg",       "video/mpeg"},
    {".mpkg",       "application/vnd.apple.installer+xml"},
    {".odp",        "application/vnd.oasis.opendocument.presentation"},
    {".ods",        "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt",        "application/vnd.oasis.opendocument.text"},
    {".oga",        "audio/ogg"},
    {".ogv",        "video/ogg"},
    {".ogx",        "application/ogg"},
    {".otf",        "font/otf"},
    {".png",        "image/png"},
    {".pdf",        "application/pdf"},
    {".ppt",        "application/vnd.ms-powerpoint"},
    {".pptx",       "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar",        "application/x-rar-compressed"},
    {".rtf",        "application/rtf"},
    {".sh",         "application/x-sh"},
    {".svg",        "image/svg+xml"},
    {".swf",        "application/x-shockwave-flash"},
    {".tar",        "application/x-tar"},
    {".tif",        "image/tiff"},
    {".tiff",       "image/tiff"},
    {".ttf",        "font/ttf"},
    {".txt",        "text/plain"},
    {".vsd",        "application/vnd.visio"},
    {".wav",        "audio/wav"},
    {".weba",       "audio/webm"},
    {".webm",       "video/webm"},
    {".webp",       "image/webp"},
    {".woff",       "font/woff"},
    {".woff2",      "font/woff2"},
    {".xhtml",      "application/xhtml+xml"},
    {".xls",        "application/vnd.ms-excel"},
    {".xlsx",       "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml",        "application/xml"},
    {".xul",        "application/vnd.mozilla.xul+xml"},
    {".zip",        "application/zip"},
    {".3gp",        "video/3gpp"},
    {".3g2",        "video/3gpp2"},
    {".7z",         "application/x-7z-compressed"}
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
        return S_ISDIR(st.st_mode);
    }
    static bool IsRegularFile(const std::string& filepath)
    {
        struct stat st;
        int ret=stat(filepath.c_str(),&st);
        if(ret==-1){ERROR_LOG("Get Filepath Stat: %s Error",filepath.c_str());return false;}
        return S_ISREG(st.st_mode);
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
    bool HasHeader(const std::string& key)const
    {
        auto it=_headers.find(key);
        if(it!=_headers.end())
        {
            return true;
        }
        return false;
    }
    std::string GetHeader(const std::string & key)const
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
    bool HasParam(const std::string& key)const
    {
        auto it =_params.find(key);
        if(it!=_params.end())
        {
            return true;
        }
        return false;
    }
    std::string GetParam(const std::string& key)const
    {
        bool ret=HasParam(key);
        if(!ret){return "";}
        return _params.find(key)->second;
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
        //如果s为空会stol会报错
        return stol(s);
    }
    
    //获取正文长度
    //判断是否短连接
    bool IsLongConnection()const
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
    bool HasHeader(const std::string& key)const
    {
        auto it=_headers.find(key);
        if(it==_headers.end())
        {
            return false;
        }
        return true;
    }
    std::string GetHeader(const std::string& key)const
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
    bool IsLongConnection()const
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
    bool RecvHttpLine( Buffer* buff)
    {
        
        // 2.接收请求行-> 获取一行数据->判断太长，太短->http报头行解析->更新状态
        if(_recv_statu!=HTTP_RECV_LINE)return false;
        DEBUG_LOG("RecvHttpLine");
        std::string httpline=buff->ReadAsLine();
        if(httpline==""){
            //判断buffer内部大小
            if(buff->ReadAbleSize()>MAXLINE)
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
    bool RecvHttpHeader(Buffer* buff)
    {
        // 3. 接收头处理
        
        if(_recv_statu!=HTTP_RECV_HEADER)return false;
        DEBUG_LOG("RecvHttpHeader");
        while(1)
        {
            std::string httpheader=buff->ReadAsLine();
            if(httpheader=="")
            {
                if(buff->ReadAbleSize()>MAXLINE)
                {
                    _statu_code=400;
                    _recv_statu=HTTP_RECV_ERROR;
                    return false;
                } 
                return true;   
            }
            if(httpheader=="\r\n"||httpheader=="\n")
            {
                break;
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

        }
        _statu_code=200;
        _recv_statu=HTTP_RECV_BODY;
        return true;

    }
    bool ParseHttpHeader( const std::string& httpheader)
    {
        // 4. 解析报头处理
        std::string header=httpheader;
        if(header.back()=='\n'){header.pop_back();}
        if(header.back()=='\r'){header.pop_back();}
        std::vector<std::string> array;
        Util::SeperateString(header,":",&array);
        if(array.size()==0){return false;}
        _request.InsertHeader(array[0],array[1]);
        return true;
    }
    bool RecvHttpBody( Buffer* buff)
    {
        // 5. 解析报文处理

        if(_recv_statu!=HTTP_RECV_BODY)return false;
        DEBUG_LOG("RecvHttpBody");
        size_t needsize=(size_t)_request.GetContentLength()-_request._body.size();
        if(needsize>buff->ReadAbleSize())
        {
            _request._body.append(buff->ReadAsString(buff->ReadAbleSize()));
            return true;
        }
        _request._body.append(buff->ReadAsString(needsize));
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
    void RecvHttp(Buffer* buff)
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
    private:
        using Handler=std::function<void(const HttpRequest&,HttpResponse*)>;
        //请求表类型
        using Handlers=std::vector<std::pair<std::regex,Handler>>;
        //1. GET POST DELETE POST 的方法的处理函数数组 接收Request 返回Response
        //2. 静态资源根目录
        //3. TcpServer
        //接收请求，返回报文函数
        Handlers _get_route;
        Handlers _post_route;
        Handlers _delete_route;
        Handlers _put_route;
        TcpServer _server;
        std::string _base_dir;
    public:
    void ErrorHandle(const HttpRequest& req,HttpResponse *rsp)
    {
        //组装错误报文
        //4. 错误处理 ->构建错误页面
        std::string body;
        body+="<html>";
        body+="<head>";
        body+="<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>";
        body+="</head>";
        body+="<h1>";
        body+=std::to_string(rsp->_status);
        body+=" ";
        body+=Util::StatusCode(rsp->_status);
        body+="</h1>";
        body+="</body>";
        body+="</html>";
        rsp->SetContent(body,"text/html");
    }
    void WriteResponse(const PtrConnection& conn,HttpResponse* rsp,const HttpRequest& req)
    {
        //报头设置
        //5. 构建响应代码 response->响应报文->缓冲区发送
        if(req.IsLongConnection()==false)
        {
            rsp->InsertHeader("Connection","close");
        }
        else{
            rsp->InsertHeader("Connection","keep-alive");
        }
        // 存在正文数据，但是不存在Content-Length报头
        if(rsp->_body.empty()==false&&rsp->HasHeader("Content-Length")==false)
        {
            rsp->InsertHeader("Content-Length",std::to_string(rsp->_body.size()));
        }
        if(rsp->_body.empty()==false&&rsp->HasHeader("Content-type")==false)
        {
            rsp->InsertHeader("Content-type","application/octet-stream");
        }
        if(rsp->_is_redirect==true)
        {
            rsp->SetRedirct(rsp->_redirect_url);
        }
        std::stringstream rsp_str;
        rsp_str<<req._version<<" "<<rsp->_status<<" "<<Util::StatusCode(rsp->_status)<<"\r\n";
        for(auto& header:rsp->_headers)
        {
            rsp_str<<header.first<<":"<<header.second<<"\r\n";
        }
        rsp_str<<"\r\n";
        rsp_str<<rsp->_body;
        rsp->Reset();
        conn->Send(rsp_str.str().c_str(),rsp_str.str().size());
    }
    //请求分为静态资源请求和功能性
    //判断是否为静态性资源请求处理
    bool IsFileHandler(const HttpRequest& req)
    {
         //6. 是否请求文件 相对根目录存在->方法限定->路径合法-> 路径变换->路径合并/自动补充->文件类型
        if(_base_dir.empty()){return false;}
        //限定方法为get和head
        if(req._method!="GET"&&req._method!="HEAD")
        {
            return false;
        }
        //路径合法 不能超出相对目录
        if(!Util::ValidePath(req._path))
        {
            return false;
        }
        // 路径为"/"或者"/image/"的补充index.html
        std::string path=req._path;
        if(path=="/"||path=="/image/")
        {
            path+="index.html";
        }
        //拼接成完整路径
        path=_base_dir+path;
        return Util::IsRegularFile(path);
    }
    void FileHandler(const HttpRequest& req,HttpResponse* rsp)
    {
        //7. 静态资源请求处理 读取文件内容
        //拼接路径
        std::string path=_base_dir+req._path;
        //路径补充
        if(path.back()=='/')
        {
            path+="index.html";
        }
        //读取文件内容
        bool ret=Util::ReadFile(path,&rsp->_body);
        if(!ret){return;}
        //Set Head Field
        rsp->InsertHeader("Content-type","text/html");
        return;
    }
    void Dispatch(const HttpRequest& req,HttpResponse* rsp,Handlers &handlers)
    {
        //遍历handlers 对 req的请求资源路径进行匹配，匹配成功则调用
        for(auto& handler:handlers)
        {
            std::regex pattern=handler.first;
            Handler functor=handler.second;
            bool ret=std::regex_match(req._path,pattern);
            if(!ret){continue;}
            return functor(req,rsp);
           
        }
        rsp->_status=404;
    }
    void Route(const HttpRequest& req,HttpResponse* rsp)
    {
        //区分功能性请求还是静态资源请求
        if(IsFileHandler(req))
        {
            FileHandler(req,rsp);
        }
        if(req._method=="GET"||req._method=="HEAD")
        {
            Dispatch(req,rsp,_get_route);
        }
        else if(req._method=="PUT")
        {
            Dispatch(req,rsp,_put_route);
        }
        else if(req._method=="POST")
        {
            Dispatch(req,rsp,_put_route);
        }
        else if(req._method=="DELETE")
        {
            Dispatch(req,rsp,_delete_route);
        }
        else{
            ERROR_LOG("Method Not Allowed");
            rsp->_status=405;
        }
        return;

    }
    //设置上下文->http Request解析状态，解析状态嘛
    void OnConnected(const PtrConnection &conn)
    {
        //传递一个匿名对象
        conn->SetContent(HttpContent());
        DEBUG_LOG("NEW CONNECTION %p",conn.get());
    }
    // 缓冲区数据解析+处理 获取上下文  根据状态码判断 路由处理 重置上下文 短连接关闭
    void OnMessage(const PtrConnection& conn,Buffer* buff){
        while(buff->ReadAbleSize()>0)//缓冲区有数据时
        {
            //获取上下文
            HttpContent& content=conn->GetContext()->any_cast<HttpContent>();
            content.RecvHttp(buff);
            HttpResponse rsp;
            //进行获取request对象
            HttpRequest& req=content.RecvRequest();
            if(content.RecvStatu()>400)
            {
                //发送错误响应
                
                ErrorHandle(req,&rsp);
                WriteResponse(conn,&rsp,req);
                //解析已经发生失败了
                content.ReSet();
                buff->RMoveOffset(buff->ReadAbleSize());
                conn->Shutdown();
                return;
            }
            //没有报错->可能没接收完->可能接收完毕
            if(content.RecvStatu()!=HTTP_RECV_OVER)
            {
                return;
            }
            //接收完毕
            Route(req,&rsp);
            WriteResponse(conn,&rsp,req);
            content.ReSet();
            if(!rsp.IsLongConnection()) conn->Shutdown();

        }
        return;
    }
    public:
        HttpServer(int port,int timeout=10):_server(port){
            _server.EnableInactiveRelease(timeout);
            _server.SetConnectedCallback(std::bind(&HttpServer::OnConnected,this,std::placeholders::_1));
            _server.SetMessageCallback(std::bind(&HttpServer::OnMessage,this,std::placeholders::_1,std::placeholders::_2));
        }
        void SetBaseDir(const std::string& path)
        {
            assert(Util::IsDirectory(path)==true);
            _base_dir=path;
        }
        void SetThreadCount(int count) {
            _server.SetThreadCount(count);
        }
        void Get(const std::string&pattern, const Handler &handler)
        {
            _get_route.push_back({std::regex(pattern),handler});
        }
        void Post(const std::string&pattern,const Handler& handler)
        {
            _post_route.push_back({std::regex(pattern),handler});
        }
        void Put(const std::string&pattern,const Handler& handler)
        {
            _put_route.push_back({std::regex(pattern),handler});
        }
        void Delete(const std::string&pattern,const Handler& handler)
        {
            _delete_route.push_back({std::regex(pattern),handler});
        }
        void Listen() {
            _server.Start();
        }
};
