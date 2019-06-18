#include "HTTPServer.h"
#include "kknet/reactor/Eventloop.h"
#include "functional"


using namespace kkwb;

HTTPServer::HTTPServer(int port)
:loop_(new kknet::Eventloop())
{
    
    server_.reset(new kknet::Server(loop_,"httpserver",80,true));
    server_->setConnectionCallback(std::bind(&HTTPServer::connectionCallback,this,std::placeholders::_1));
    server_->setMessageCallback(std::bind(&HTTPServer::messageCallback,this,std::placeholders::_1));
    server_->setCloseCallback(std::bind(&HTTPServer::closeConnectionCallback,this,std::placeholders::_1));
}

HTTPServer::~HTTPServer()
{

}

void HTTPServer::closeConnectionCallback(const kknet::ConnectionPtr& conn)
{
    connectionMap_[conn->getFd()]->clearTimer();
    connectionMap_.erase(conn->getFd());
}

void HTTPServer::start()
{
    server_->setThreadNum(4);
    server_->start();
    loop_->loop();
}

void HTTPServer::connectionCallback(const kknet::ConnectionPtr& conn)
{
     //printf("%s已经连接，IP%s\n",conn->getName().c_str(),conn->getPeerAddress().getIpString().c_str());
     connectionMap_[conn->getFd()] = HTTPConnectionPtr(new HTTPConnection(conn));
}

void HTTPServer::messageCallback(const kknet::ConnectionPtr& conn)
{
    connectionMap_[conn->getFd()]->handleRequest(conn->getInputBuffer());

}



