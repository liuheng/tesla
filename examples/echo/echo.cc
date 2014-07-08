#include "echo.h"

#include <tesla/base/Logger.h>

#include <boost/bind.hpp>

using namespace tesla::base;
// using namespace tesla;
// using namespace tesla::net;

EchoServer::EchoServer(tesla::net::EventLoop* loop,
                       const tesla::net::InetAddress& listenAddr)
    : server_(loop, listenAddr, "EchoServer")
{
    server_.setConnectionCallback(
        boost::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
    server_.start();
}

void EchoServer::onConnection(const tesla::net::TcpConnectionPtr& conn)
{
    LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const tesla::net::TcpConnectionPtr& conn,
                           tesla::net::Buffer* buf,
                           tesla::base::Timestamp time)
{
    tesla::base::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
             << "data received at " << time.toString();
    conn->send(msg);
}

