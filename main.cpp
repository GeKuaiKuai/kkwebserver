#include "HTTPServer.h"
#include "kknet/net/EventLoop.h"
#include "kknet/net/Client.h"
#include "kknet/net/InetAddress.h"
#include <functional>


int main()
{
    kkwb::HTTPServer server;
    server.start();
    /*
    kknet::EventLoop loop;
    kknet::InetAddress serverAddr("127.0.0.1",9000);
    kknet::Client client(&loop,serverAddr,string("Client"));
    client.connect();
    client.setConnectionCallback(std::bind(&newConnection,std::placeholders::_1));
    client.setMessageCallback(std::bind(&messageHnadle,std::placeholders::_1,std::placeholders::_2));
    loop.loop();
    */
    
    
    return 0;
}