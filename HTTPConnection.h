#ifndef KKWB_HTTPCONNECTION_h
#define KKWB_HTTPCONNECTION_h
#include "kknet/net/Connection.h"
#include "kknet/net/Buffer.h"
#include "FastCGI.h"
#include "RequestParse.h"
#include "Response.h"
#include <memory>
#define MAX_READ_SIZE 1024*5
namespace kkwb
{
    struct FileData
    {
        string url;
        uint64_t offset;
        uint64_t length;
        int fd;
    };


    class HTTPConnection:public std::enable_shared_from_this<HTTPConnection>
    {
        public:
            HTTPConnection(const kknet::ConnectionPtr& conn):conn_(conn),step_(kversion)
            {
                reset();
            }
            void handleRequest(kknet::Buffer* buffer);
            kknet::ConnectionPtr getConn()
            {
                return conn_;
            }
            void setDataFromCGI(const char* data,size_t len)
            {
                data_ += string(data,len);
            }
            
            //解析CGI发来的消息
            void getcgi();
            const string& getURL()
            {
                return request_->url;
            }
            const string& getMethod()
            {
                return request_->method;
            }
            const string& getQuery()
            {
                return request_->query;
            }
            const string& getBody()
            {
                return request_->body;
            }
            const string& getHTTPHead(const string& key)
            {
                return request_->headMap[key];
            }
            const std::map<string,string>& getHTTPHeads()
            {
                return request_->headMap;
            }
            const string& getFileURL()
            {
                return request_->fileurl;
            }
        private:
            void response();
            void clearFileData();
            void newFileTask(const string &url);
            void sendFile();
            //emptyLine 如果为false,不发送\r\n空行
            void sendHead(bool emptyLine=true);
            void reset();
            void handleResponse();
            void sendcgi();
            
            void sendData(const char*pos,size_t length);
            //向客户端发送错误信息
            void sendError();
            //生成错误信息
            static string makeErrorMessage(const string& state);
            enum HandleStep
            {
                kversion,
                khead,
                kbody,
                kresponse,
                ksendcgi,
                kgetcgi,
                ksenddata,
                ksendfile,
                ksenderror,
                kwait
            };
            kknet::ConnectionPtr conn_;
            char step_;
            std::unique_ptr<Request> request_;
            FileData filedata_;
            std::unique_ptr<Response> response_;
            std::unique_ptr<FastCGI> fastcgi_;
            //CGI数据 or 异常信息
            string data_;

    };
}

#endif