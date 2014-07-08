#include "codec.h"

#include <tesla/base/Logger.h>
#include <tesla/base/Mutex.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/EventLoop.h>
#include <tesla/net/TcpServer.h>

#include <boost/bind.hpp>

#include <set>
#include <stdio.h>

using namespace tesla::base;
using namespace tesla::net;

class ChatServer
    : private Noncopyable
{
public:
    ChatServer(EventLoop* loop,
               const InetAddress& listenAddr)
        : server_(loop, listenAddr, "ChatServer"),
          codec_(boost::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
    {
        server_.setConnectionCallback(
            boost::bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(
            boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    }

    void start()
    {
        server_.start();
    }

private:
    typedef std::set<TcpConnectionPtr> ConnectionList;

    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
                 << conn->peerAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected())
        {
            connections_.insert(conn);
        }
        else
        {
            connections_.erase(conn);
        }
    }

    void onStringMessage(const TcpConnectionPtr&,
                         const string& message,
                         Timestamp)
    {
        for (ConnectionList::iterator it = connections_.begin();
                it != connections_.end();
                ++it)
        {
            codec_.send(get_pointer(*it), message);
        }
    }

    TcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 1)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else
    {
        printf("Usage: %s port\n", argv[0]);
    }
}

