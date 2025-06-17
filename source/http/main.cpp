#include"http.hpp"
// void TestUtil()
// {
//     Util u;
//     std::string str="Test Write File";
//     std::string pathname="./file.txt";
//     Util::WriteFile(pathname,str);
//     std::string str1;
//     Util::ReadFile(pathname,&str1);
//     std::cout<<str1<<std::endl;
//     std::cout<<Util::IsRegularFile(pathname)<<std::endl;
//     std::cout<<Util::IsDirectory(pathname)<<std::endl;

//     std::cout<<Util::StatusCode(201)<<std::endl;
//     std::cout<<Util::MimeCode(pathname)<<std::endl;
//     std::cout<<Util::ValidePath(pathname+="/../..")<<std::endl;
//     std::string rawstr="Hello World!@测试";
//     std::string url=Util::EncodeUrl(rawstr);
//     std::cout<<url<<std::endl;
//     url=Util::DecodeUrl(url);
//     std::cout<<url<<std::endl;
// }
// void TestHttpRequest()
// {
//     HttpRequest request;
//     std::string body("<!DOCTYPE html><html>hellow word!</html>");
//     std::string httpr="GET /dir1/dir2/index.html HTTP/1.1\r\nContent-Length:"+std::to_string(body.size())+"\r\nConnection:keep-alive\r\n"+body;
//     std::string filepath="./httprequest.txt";
//     Util::WriteFile(filepath,httpr);
//     //报文
//     std::string rev;
//     Util::ReadFile(filepath,&rev);
//     std::vector<std::string> array;
//     size_t size=Util::SeperateString(rev,"\r\n",&array);
//     for(int i=0;i<size-1;i++)
//     {
//         if(i==0)
//         {
//             std::vector<std::string> array1;
//             size_t size1=Util::SeperateString(array[0]," ",&array1);
//             request._method=array1[0];
//             request._path=array1[1];
//             request._version=array1[2];
//             continue;
//         }
//         std::vector<std::string> header_array;
//         Util::SeperateString(array[i],":",&header_array);
//         request.InsertHeader(header_array[0],header_array[1]);
//     }
//     request._body=array[size-1];
//     std::cout<<request._method<<std::endl;
//     std::cout<<request._path<<std::endl;
//     std::cout<<request._version<<std::endl;
//     for(auto& e: request._headers)
//     {
//         std::cout<<e.first<<":"<<e.second<<std::endl;
//     }
//     std::cout<<request._body<<std::endl;
//     std::cout<<"Content-Length"<<request.GetContentLength()<<std::endl;
//     std::cout<<"Connection"<<request.IsLongConnection()<<std::endl;
//     request.Reset();
// }
// void TestRecvRequest()
// {
//     std::string body("<!DOCTYPE html><html>hellow word!</html>");
//     std::string httpr="GET /dir1/dir2/index.html HTTP/1.1\r\nContent-Length:"+std::to_string(body.size())+"\r\nConnection:keep-alive\r\n\r\n"+body;
//     Buffer inbuff;
//     inbuff.WriteAsString(httpr);
//     HttpContent hc;
//     hc.RecvHttp(&inbuff);
//     HttpRequest request=hc.RecvRequest();
//     std::cout<<hc.StatuCode()<<std::endl;
//     std::cout<<request._method<<std::endl;
//     std::cout<<request._path<<std::endl;
//     std::cout<<request._version<<std::endl;
//     for(auto& e: request._headers)
//     {
//         std::cout<<e.first<<":"<<e.second<<std::endl;
//     }
//     std::cout<<request._body<<std::endl;
//     std::cout<<"Content-Length: "<<request.GetContentLength()<<std::endl;
//     std::cout<<"Connection: "<<request.IsLongConnection()<<std::endl;
// }
// void RegularHttpHeader()
// {
//     std::string url1="GET /dir1/dir2/a.html?a=1&b=2&c=3 HTTP/1.1\r\n";
//     std::string url2="POST /index.html HTTP/1.1\r\n";
//     std::string url3="POST /index.html/a.html?a=1&b=2&c=3 \r\n";
//     std::string url4="POST /index.html?a=1&b=2&c=3 HTTP/1.1\n";
//     std::smatch match1;
//     std::smatch match2;
//     std::smatch match3;
//     std::smatch match4;

//     std::regex pattern1("(GET|POST|PUT|DELETE) ([^\\s?]+)(?:\\?([^\\s]+))? (HTTP/\\d\\.\\d)?\r?\n");
//     std::regex_match(url1,match1,pattern1);
//     for(auto e:match1)
//     {
//         std::cout<<e<<" ";
//     }
//     std::cout<<std::endl;
//     std::regex_match(url2,match2,pattern1);
//     for(auto e:match2)
//     {
//         std::cout<<e<<" ";
//     }
//     std::cout<<std::endl;
//     std::regex_match(url3,match3,pattern1);
//     for(auto e:match3)
//     {
//         std::cout<<e<<" ";
//     }
//     std::cout<<std::endl;
//     std::regex_match(url4,match4,pattern1);
//     for(auto e:match4)
//     {
//         std::cout<<e<<" ";
//     }
//     std::cout<<std::endl;

// }
#define WWWROOT "./wwwroot/"

std::string RequestStr(const HttpRequest &req) {
    std::stringstream ss;
    ss << req._method << " " << req._path << " " << req._version << "\r\n";
    for (auto &it : req._params) {
        ss << it.first << ": " << it.second << "\r\n";
    }
    for (auto &it : req._headers) {
        ss << it.first << ": " << it.second << "\r\n";
    }
    ss << "\r\n";
    ss << req._body;
    return ss.str();
}
void Hello(const HttpRequest &req, HttpResponse *rsp) 
{
    rsp->SetContent(RequestStr(req), "text/plain");
}
void Login(const HttpRequest &req, HttpResponse *rsp) 
{
    rsp->SetContent(RequestStr(req), "text/plain");
}
void PutFile(const HttpRequest &req, HttpResponse *rsp) 
{
    std::string pathname = WWWROOT + req._path;
    Util::WriteFile(pathname, req._body);
}
void DelFile(const HttpRequest &req, HttpResponse *rsp) 
{
    rsp->SetContent(RequestStr(req), "text/plain");
}
int main()
{
    HttpServer server(8085);
    server.SetThreadCount(3);
    server.SetBaseDir(WWWROOT);//设置静态资源根目录，告诉服务器有静态资源请求到来，需要到哪里去找资源文件
    server.Get("/hello", Hello);
    server.Post("/login", Login);
    server.Put("/1234.txt", PutFile);
    server.Delete("/1234.txt", DelFile);
    server.Listen();
    return 0;
}