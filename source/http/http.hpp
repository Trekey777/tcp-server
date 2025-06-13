
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

    static std::string EncodeUrl(const std::string& rawstr)
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
            if(s==' ')
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
    static std::string DecodeUrl(const std::string& url )
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
            if(s=='+')
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
    std::smatch _match;//正则表达式结果
    std::unordered_map<std::string,std::string> _headers;
    std::unordered_map<std::string,std::string> _params;
    public:
    HttpRequest():_version("HTTP/1.1"){}
    void Reset()
    {
        _method.clear();
        _path.clear();
        _version="HTTP/1.1";
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
    std::string GetContentLength()
    {
        std::string s=GetHeader("Content-Length");
        return s;
    }
    
    //获取正文长度
    //判断是否短连接
    bool GetConnection()
    {
        std::string s=GetHeader("Connection");
        if(s=="keep-alive"){
            return true;
        }
        return false;
    }
};