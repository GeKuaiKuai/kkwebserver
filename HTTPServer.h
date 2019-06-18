#ifndef KKWB_HTTPSERVER_H
#define KKWB_HTTPSERVER_H

#include "kknet/reactor/Server.h"
#include "HTTPConnection.h"
#include "map"

namespace kkwb
{
    class HTTPServer
    {
        public:
            HTTPServer(int port = 80);
            ~HTTPServer();
            void start();
            void stop();
        private:
            typedef std::shared_ptr<HTTPConnection> HTTPConnectionPtr;
            void connectionCallback(const kknet::ConnectionPtr&);
            void messageCallback(const kknet::ConnectionPtr&);
            void closeConnectionCallback(const kknet::ConnectionPtr&);
            std::unique_ptr<kknet::Server> server_;
            kknet::Eventloop* loop_;
            std::map<int,HTTPConnectionPtr> connectionMap_;

    };
}



#endif