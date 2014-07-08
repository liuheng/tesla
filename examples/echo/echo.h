#ifndef TESLA_EXAMPLES_SIMPLE_ECHO_ECHO_H
#define TESLA_EXAMPLES_SIMPLE_ECHO_ECHO_H

#include <tesla/net/TcpServer.h>

// RFC 862
class EchoServer
{
public:
    EchoServer(tesla::net::EventLoop* loop,
               const tesla::net::InetAddress& listenAddr);

    void start();  // calls server_.start();

private:
    void onConnection(const tesla::net::TcpConnectionPtr& conn);

    void onMessage(const tesla::net::TcpConnectionPtr& conn,
                   tesla::net::Buffer* buf,
                   tesla::base::Timestamp time);

    tesla::net::TcpServer server_;
};

#endif  // TESLA_EXAMPLES_SIMPLE_ECHO_ECHO_H
