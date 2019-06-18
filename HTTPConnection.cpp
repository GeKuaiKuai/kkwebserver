#include "HTTPConnection.h"
#include "kknet/util/typedef.h"
#include "MemoryCache.h"
#include "kknet/reactor/Connection.h"
#include "kknet/reactor/Eventloop.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "functions.h"
using namespace kkwb;



void HTTPConnection::handleRequest(kknet::KBuffer* buffer)
{
    char buf[RESPONSE_LINE_LENGTH];
    memInit(buf,sizeof buf);
    if(step_==kversion||step_==khead)
    {
        while(buffer->getReadableSize()!=0 && buffer->getLine(buf))
        {
            
            switch(step_)
            {
                case kversion:
                    versionParse(buf,request_);
                    step_ = khead;
                    break;
                case khead:
                    if(!headParse(buf,request_))
                    {
                        step_ = kbody;
                    }
                        
                    break;
            }
            if(step_ == kbody)
                break;
            memInit(buf,sizeof buf);
        }
    }
    
    if(step_ == kbody)
    {
        if(!getHTTPHead("content-length").empty())
        {       
            //todo 改成分块读取，实现大文件上传
            size_t l = stringToNum<size_t>(getHTTPHead("content-length"));
            if(buffer->getReadableSize()>=l)
            {
                char body_buf[RESPONSE_LENGTH];
                memInit(body_buf,sizeof body_buf);
                buffer->read(body_buf,l);
                request_->body = string(body_buf);
                step_ = kresponse;
            }
        }
        else
            step_ = kresponse;
        
    }

    if(step_ == kresponse)
        conn_->getLoop()->queueInLoop(std::bind(&HTTPConnection::handleResponse,shared_from_this()));

}

void HTTPConnection::handleResponse()
{

    switch(step_)
    {
        case kresponse:
            response();
            break;
        case ksenderror:
            sendError();
            break;
        case ksendcgi:
            sendcgi();
            break;
        case ksendfile:
            sendFile();
            break;
    }

    //发送文件需要自循环
    if(step_ != kversion && step_ != kwait)
    {
        conn_->getLoop()->queueInLoop(std::bind(&HTTPConnection::handleResponse,shared_from_this()));
    }
}
void HTTPConnection::response()
{
    
    //连接计时器
    clearTimer();
    connTimer_ = conn_->getLoop()->runAfter(120,std::bind(&HTTPConnection::close,shared_from_this()));
    

    response_->setVersion();
    //处理网站根目录
    string url = "/home/ak644928/wwwroot"+request_->url;
    
    if(isDIR(url))
    {
        if(url[url.size()-1] != '/')
        {
            response_->setState(HTTP_301);
            response_->addHead("location","http://"+getHTTPHead("host")+request_->url+"/");
            sendError();
            return;
        }
        //处理默认首页
        if(isHaveFile(url+"index.php"))
        {
            url+="index.php";
            request_->url+="index.php";
        }
        else if(isHaveFile(url+"index.html"))
        {
            url+="index.html";
            request_->url+="index.html";
        }
        //禁止访问目录
        else
        {
            response_->setState(HTTP_STATE_FORBIDDEN);
            sendError();
            return;
        }
    }
    
    if(isHaveFile(url))
    {
        response_->setState();   
    }
    else
    {
        MemoryCache::getInstance()->lock();
        MemoryCache::getInstance()->deleteNode(url);
        MemoryCache::getInstance()->unlock();
        response_->setState(HTTP_STATE_NOTFOUND);
        //处理404页面
        if(false)
            url = "/home/ak644928/wwwroot/index.html";
        else
        {
            step_ = ksenderror;
            return;
        }
    }

    string extendName = getExtendName(url);

    

    if(extendName == ".php")
    {
        
        //cgi需要用到fileurl和url
        request_->fileurl = url;
        step_ = ksendcgi;
        return;
    }
    else
    {

        newFileTask(url);
        response_->addHead(HTTP_CONTENT_TYPE,getMIME(extendName));
        char num[HOST_LENGTH] = {0};
        snprintf(num,sizeof num,"%ld",filedata_.length);
        response_->addHead(HTTP_LENGTH,num);
        sendHead();
        step_ = ksendfile;
        
    }
    


}

void HTTPConnection::sendHead(bool emptyLine)
{
    //基本信息
    response_->addHead(HTTP_SERVER,HTTP_SERVER_NAME);
    if(request_->headMap["connection"] == "close"||request_->headMap["connection"] == "Close")
        response_->addHead("Connection","close");
    else
        response_->addHead("Connection","Keep-Alive");

    char buf[RESPONSE_LENGTH];
    int index = 0;
    int valueIndex = 0;
    const string& version = response_->getVersion();
    const string& state = response_->getState();
    //写入HTTP版本
    ::memcpy(buf+index,version.c_str(),version.size());
    index+=version.size();
    ::memcpy(buf+index," ",1);
    index+=1;
    //写入HTTP状态
    ::memcpy(buf+index,state.c_str(),state.size());
    index+=state.size();
    ::memcpy(buf+index,"\r\n",2);
    index+=2;
    //写入头部字段
    for(auto &head:response_->getHeads())
    {
        if(!head.second.empty())
        {
            valueIndex = 0;
            ::memcpy(buf+index,head.first.c_str(),head.first.size());
            index+=head.first.size();
            ::memcpy(buf+index,": ",2);
            index+=2;
            for(auto &value:head.second)
            {
                ::memcpy(buf+index,value.c_str(),value.size());
                index+=value.size();
                valueIndex++;
                if(valueIndex < head.second.size())
                {
                    ::memcpy(buf+index,", ",2);
                    index+=2;
                }
            }
            ::memcpy(buf+index,"\r\n",2);
            index+=2;  
        }

      
    }
    if(emptyLine)
    {
        ::memcpy(buf+index,"\r\n",2);
        index+=2;
    }

    conn_->send(buf,index);
}

void HTTPConnection::clearFileData()
{
    filedata_.url.clear();
    filedata_.fd = -1;
    filedata_.length = 0;
    filedata_.offset = 0;
    filedata_.data.clear();
}

void HTTPConnection::newFileTask(const string &url)
{
    clearFileData();
    filedata_.url = url;
    filedata_.offset = 0;
    struct stat buf;
    ::stat(url.c_str(),&buf);
    filedata_.length = buf.st_size;
    timespec time = buf.st_mtim;
    
    if(buf.st_size <= MEMORY_CACHE_MAXFILESIZE)
    {
        MemoryCache::getInstance()->lock();
        CacheNode* node  = MemoryCache::getInstance()->getNode(url);
        
        if(!node || node->time.tv_sec != time.tv_sec || node->time.tv_nsec != time.tv_sec)
        {
            char databuf[MEMORY_CACHE_MAXFILESIZE] = {0};
            int fd = ::open(url.c_str(),O_RDONLY|O_CLOEXEC);
            size_t n = ::read(fd,databuf,MEMORY_CACHE_MAXFILESIZE);
            
            ::close(fd);
            node  = new CacheNode();
            node->data = string(databuf,n);
            node->url = url;
            node->time.tv_sec = time.tv_sec;
            node->time.tv_nsec = time.tv_nsec;
            node->length = buf.st_size;
            MemoryCache::getInstance()->pushNode(url,node);
        }

        filedata_.data = MemoryCache::getInstance()->getData(url);
        MemoryCache::getInstance()->unlock();
    }
    else
    {
        filedata_.fd = ::open(url.c_str(),O_RDONLY|O_CLOEXEC);
    }
    
    

}

void HTTPConnection::sendFile()
{
    
    size_t n = 0;
    if(filedata_.data.empty())
    {
        char buf[MAX_READ_SIZE] = {0};
        n = ::read(filedata_.fd,buf,sizeof buf);
        conn_->send(buf,n);
    }       
    else
    {
        string buf = filedata_.data.substr(filedata_.offset,MAX_READ_SIZE);
        //printf("%ld %ld %ld\n",filedata_.data.size(),filedata_.offset,filedata_.length);
        n = buf.size();
        conn_->send(buf.c_str(),buf.size()); 
    }
    filedata_.offset += n;
    
    if(filedata_.length == filedata_.offset)
    {
        
        if(filedata_.data.empty())
        {
            ::close(filedata_.fd);
        }
        reset();
    }
}

void HTTPConnection::reset()
{

    if(step_!=kversion && (request_->headMap["connection"] == "close"||request_->headMap["connection"] == "Close"))
    {
        conn_->shutdown();
        return;
    }
    request_.reset(new Request());
    response_.reset(new Response());
    fastcgi_.reset();
    data_.clear();
    step_ = kversion;
        
    
}

void HTTPConnection::close()
{
    //清空连接计时器
    clearTimer();
    conn_->forceClose();
}

void HTTPConnection::sendcgi()
{
    fastcgi_.reset(new FastCGI(shared_from_this()));
    step_ = kwait;
    
}

void HTTPConnection::getcgi()
{

    string key;
    string value;
    auto begin = data_.begin();
    auto end = data_.begin();
    auto start = data_.begin();
    size_t pos = 0;

    while(true)
    {
        while(*end!='\n') end++;
        if(headParse(data_.c_str()+(begin-start),key,value))
        {
            response_->deleteHead(key);
            end++;
            begin = end;
            if(turnSmallLetter(key) == "status" )
            {
                pos = end-start;
                response_->setState(value);
            }
        }
        else
        {
            char num[HOST_LENGTH] = {0};
            snprintf(num,sizeof num,"%ld",data_.size()-(end-start+1));
            response_->addHead(HTTP_LENGTH,num);
            break;
        }
            
    }

    sendHead(false);
    sendData(data_.c_str()+pos,data_.length()-pos);
}


void HTTPConnection::sendData(const char* pos,size_t length)
{
    conn_->send(pos,length);
    reset();
}


void HTTPConnection::sendError()
{
    data_ = makeErrorMessage(response_->getState());
    response_->addHead(HTTP_CONTENT_TYPE,"text/html");
    char num[HOST_LENGTH] = {0};
    snprintf(num,sizeof num,"%ld",data_.size());
    response_->addHead(HTTP_LENGTH,num);    
    sendHead();
    sendData(data_.c_str(),data_.length());
}

string HTTPConnection::makeErrorMessage(const string& state)
{
    char buf[RESPONSE_LENGTH] = {0};
    int index = 0;
    ::memcpy(buf+index,HTTP_ERROR_MESS1,sizeof(HTTP_ERROR_MESS1)-1);
    index+=sizeof(HTTP_ERROR_MESS1)-1;
    ::memcpy(buf+index,state.c_str(),state.size());
    index+=state.size();
    
    ::memcpy(buf+index,HTTP_ERROR_MESS2,sizeof(HTTP_ERROR_MESS2)-1);
    index+=sizeof(HTTP_ERROR_MESS2)-1;  
    ::memcpy(buf+index,state.c_str(),state.size());
    index+=state.size();
    ::memcpy(buf+index,HTTP_ERROR_MESS3,sizeof(HTTP_ERROR_MESS3)-1);
    index+=sizeof(HTTP_ERROR_MESS3)-1;
    ::memcpy(buf+index,HTTP_SERVER_NAME,sizeof(HTTP_SERVER_NAME)-1);
    index+=sizeof(HTTP_SERVER_NAME)-1;
    ::memcpy(buf+index,HTTP_ERROR_MESS4,sizeof(HTTP_ERROR_MESS4)-1);
    index+=sizeof(HTTP_ERROR_MESS4)-1;
    return buf;

}