#include "FastCGI.h"
#include "HTTPConnection.h"
#include "functions.h"
using namespace kkwb;

FastCGI::FastCGI(const HTTPConnectionPtr& conn)
:HTTPConn_(conn)
{
    kknet::InetAddress serverAddr("127.0.0.1",9000);
    loop_ = conn->getConn()->getLoop();
    client_.reset(new kknet::Client(loop_,serverAddr,"FastCGI"));
    client_->connect();
    client_->setConnectionCallback(std::bind(&FastCGI::newConnection,this,std::placeholders::_1));
    client_->setMessageCallback(std::bind(&FastCGI::messageHnadle,this,std::placeholders::_1,std::placeholders::_2));    
}

void FastCGI::newConnection(kknet::ConnectionPtr conn)
{
    ngx_http_fastcgi_header_t header = {
        1,
        NGX_HTTP_FASTCGI_BEGIN_REQUEST,
        0,
        1,
        0,
        sizeof(ngx_http_fastcgi_begin_request_t),
        0,
        0
    };

    u_char padding = 8 - sizeof(ngx_http_fastcgi_begin_request_t)%8;
    padding = (padding == 8) ? 0 : padding;
    header.padding_length = padding;
    
    ngx_http_fastcgi_begin_request_t begin = 
    {
        0,1,0,{0,0,0,0,0}
    };
    
    conn->send(&header,sizeof header+padding);
    conn->send(&begin,sizeof begin);

    char buf[10240] = {0};
    size_t index = 0;

    addParam("REQUEST_METHOD",HTTPConn_->getMethod());
    
    if(HTTPConn_->getMethod() == "POST")
    {
        string l = HTTPConn_->getHTTPHead("content-length");
        addParam("CONTENT_LENGTH",l);
    }
    else
    {
        addParam("CONTENT_LENGTH","0");
    }
    
    addParam("QUERY_STRING",HTTPConn_->getQuery());

    addParam("SCRIPT_NAME",HTTPConn_->getURL());
    addParam("SCRIPT_FILENAME",HTTPConn_->getFileURL());

    addParam("SERVER_SOFTWARE",HTTP_SERVER_NAME);
    if(HTTPConn_->getHTTPHead("content-type").empty())  
        addParam("CONTENT_TYPE","application/x-www-form-urlencoded");
    else
        addParam("CONTENT_TYPE",HTTPConn_->getHTTPHead("content-type"));

    makeParam(buf,index);
    
    u_char hi = static_cast<u_char>((index >> 8) & 0xff);
    u_char lo = static_cast<u_char>(index & 0xff);
    ngx_http_fastcgi_header_t header1 = {
        1,
        NGX_HTTP_FASTCGI_PARAMS,
        0,
        1,
        hi,
        lo,
        0,
        0
    };

    padding = 8 - index%8;
    padding = (padding == 8) ? 0 : padding;
    header1.padding_length = padding;
    
    conn->send(&header1,sizeof header1);
    conn->send(&buf,index+padding);
    
    
    ngx_http_fastcgi_header_t header2 = {
        1,
        NGX_HTTP_FASTCGI_PARAMS,
        0,
        1,
        0,
        0,
        0,
        0
    };
    conn->send(&header2,sizeof header2);

    ngx_http_fastcgi_header_t header3 = {
        1,
        NGX_HTTP_FASTCGI_STDIN,
        0,
        1,
        hi,
        lo,
        0,
        0
    };
    
    index = 0;
    size_t left = HTTPConn_->getBody().length();
    size_t size = 0;

    while(left >0)
    {
        if(left>65535)
            size = 65535;
        else
            size = left;
        
        hi = static_cast<u_char>((size >> 8) & 0xff);
        lo = static_cast<u_char>(size & 0xff);
        header3.content_length_hi = hi;
        header3.content_length_lo = lo;
        
        conn->send(&header3,sizeof header3);
        conn->send(HTTPConn_->getBody().c_str()+index,size);
        left -= size;
        index += size;
        
    }
    header3.content_length_hi = 0;
    header3.content_length_lo = 0;
    conn->send(&header3,sizeof header3);
}
void FastCGI::messageHnadle(kknet::ConnectionPtr conn,kknet::Buffer* buffer)
{
    ngx_http_fastcgi_header_t header;
    while(buffer->readableBytes() >= sizeof header)
    {
        char buf[RESPONSE_LENGTH] = {0};

        buffer->peek(&header,sizeof header);
        size_t len = (header.content_length_hi << 8) + header.content_length_lo;

        if(buffer->readableBytes() - sizeof header < len+header.padding_length)
        {
            return;
        }        
        buffer->retrieve(sizeof header);
        buffer->read(&buf,len);    
        buffer->retrieve(header.padding_length);

        if(header.type == NGX_HTTP_FASTCGI_STDOUT)
        {
            HTTPConn_->setDataFromCGI(buf,len);
        }
        else if(header.type == NGX_HTTP_FASTCGI_END_REQUEST)
        {
            loop_->queueInLoop(std::bind(&HTTPConnection::getcgi,HTTPConn_));
        }
    }
    

}

void FastCGI::makeParam(char* buf,size_t& index)
{
    const std::map<string,string>& heads = HTTPConn_->getHTTPHeads();
    for(auto &h:heads)
    {
        addParam(h.first,h.second,true);
    }
    
    size_t len;
    
    for(auto &p:param_)
    {
        if(!p.second.empty())
        {
            len = p.first.size();
            if(len>127)
            {
                buf[index++] = (u_char) (((len >> 24) & 0x7f) | 0x80);
                buf[index++] = (u_char) ((len >> 16) & 0xff);
                buf[index++] = (u_char) ((len >> 8) & 0xff);
                buf[index++] = (u_char) (len & 0xff);
            }
            else
            {
                buf[index++] = len;
            }

            len = p.second.size();
            if(len>127)
            {
                buf[index++] = (u_char) (((len >> 24) & 0x7f) | 0x80);
                buf[index++] = (u_char) ((len >> 16) & 0xff);
                buf[index++] = (u_char) ((len >> 8) & 0xff);
                buf[index++] = (u_char) (len & 0xff);
            }
            else
            {
                buf[index++] = len;
            }

            ::memcpy(buf+index,p.first.c_str(),p.first.size());
            index+=p.first.size();
            ::memcpy(buf+index,p.second.c_str(),p.second.size());
            index+=p.second.size();
        }
    }

}

void FastCGI::addParam(const string& k,const string& value,bool http)
{
    string key = k;
    if(http)
    {
        for(auto &ch:key)
        {
            if(ch >= 'a' && ch <= 'z')
            {
                ch &= ~0x20;
            }
            else if(ch == '-')
            {
                ch = '_';
            }
        }
        param_["HTTP_"+key] = value;
    }
    else
    {
        param_[key] = value;
    }
}