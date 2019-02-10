#include "HTTPServer.h"
#include "kknet/net/EventLoop.h"
#include "functional"

using namespace kkwb;

HTTPServer::HTTPServer(int port)
:loop_(new kknet::EventLoop())
{
    kknet::InetAddress addr(80);
    server_.reset(new kknet::Server(loop_,addr,"httpserver",true));
    server_->setConnectionCallback(std::bind(&HTTPServer::connectionCallback,this,std::placeholders::_1));
    server_->setMessageCallback(std::bind(&HTTPServer::messageCallback,this,std::placeholders::_1,std::placeholders::_2));
    
}

HTTPServer::~HTTPServer()
{

}

void HTTPServer::start()
{
    server_->setThreadNum(2);
    server_->start();
    loop_->loop();
}

void HTTPServer::connectionCallback(const kknet::ConnectionPtr& conn)
{
     printf("%s已经连接，IP%s\n",conn->getName().c_str(),conn->getPeerAddress().getIpString().c_str());
     connectionMap_[conn->getName()] = HTTPConnectionPtr(new HTTPConnection(conn));
}

void HTTPServer::messageCallback(const kknet::ConnectionPtr& conn,kknet::Buffer* buffer)
{
    connectionMap_[conn->getName()]->handleRequest(buffer);
}



