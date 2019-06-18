#include "HTTPServer.h"
#include "kknet/reactor/Eventloop.h"
#include "kknet/reactor/Client.h"
#include <functional>


int main()
{
    kkwb::HTTPServer server;
    server.start();
    
    
    return 0;
}