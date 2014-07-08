#include "echo.h"

#include <tesla/base/Logger.h>
#include <tesla/net/EventLoop.h>

using namespace tesla::base;
// using namespace tesla;
// using namespace tesla::net;

int main()
{
    Logger::setLogLevel(Logger::TRACE);
    LOG_INFO << "pid = " << getpid();
    tesla::net::EventLoop loop;
    tesla::net::InetAddress listenAddr(2007);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}

