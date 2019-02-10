#ifndef KKWB_FASTCGI_H
#define KKWB_FASTCGI_H
#include "kknet/net/Client.h"
#include "kknet/net/EventLoop.h"
#include "kknet/net/InetAddress.h"
#include <map>

#include <memory>

#define NGX_HTTP_FASTCGI_BEGIN_REQUEST  1
#define NGX_HTTP_FASTCGI_ABORT_REQUEST  2
#define NGX_HTTP_FASTCGI_END_REQUEST    3
#define NGX_HTTP_FASTCGI_PARAMS         4
#define NGX_HTTP_FASTCGI_STDIN          5
#define NGX_HTTP_FASTCGI_STDOUT         6
#define NGX_HTTP_FASTCGI_STDERR         7
#define NGX_HTTP_FASTCGI_DATA           8

typedef struct {
    u_char  version; //FastCGI协议版本
    u_char  type;    //消息类型
    u_char  request_id_hi; //请求ID
    u_char  request_id_lo;
    u_char  content_length_hi; //内容
    u_char  content_length_lo;
    u_char  padding_length;    //内容填充长度
    u_char  reserved;          //保留
} ngx_http_fastcgi_header_t;

typedef struct {
    u_char  role_hi; //标记FastCGI应用应该扮演的角色
    u_char  role_lo;
    u_char  flags;
    u_char  reserved[5];
} ngx_http_fastcgi_begin_request_t;

namespace kkwb
{
    class HTTPConnection;
    class FastCGI
    {
        public:
            typedef std::shared_ptr<HTTPConnection> HTTPConnectionPtr;
            FastCGI(const HTTPConnectionPtr& conn);
        private:
            //增加参数
            void addParam(const string& key,const string& value,bool http=false);
            void makeParam(char* buf,size_t& index);
            void newConnection(kknet::ConnectionPtr conn);
            void messageHnadle(kknet::ConnectionPtr conn,kknet::Buffer* buffer);
            std::unique_ptr<kknet::Client> client_;
            kknet::EventLoop* loop_;
            HTTPConnectionPtr HTTPConn_;
            std::map<string,string> param_;
    };
}

#endif