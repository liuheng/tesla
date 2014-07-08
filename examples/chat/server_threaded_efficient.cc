#include "codec.h"

#include <tesla/base/Logger.h>
#include <tesla/base/Mutex.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/EventLoop.h>
#include <tesla/net/TcpServer.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

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
          codec_(boost::bind(&ChatServer::onStringMessage, this, _1, _2, _3)),
          connections_(new ConnectionList)
    {
        server_.setConnectionCallback(
            boost::bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(
            boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    }

    void setThreadNum(int numThreads)
    {
        server_.setThreadNum(numThreads);
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
                 << conn->peerAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");

        MutexLockGuard lock(mutex_);
        if (!connections_.unique())
        {
            connections_.reset(new ConnectionList(*connections_));
        }
        assert(connections_.unique());

        if (conn->connected())
        {
            connections_->insert(conn);
        }
        else
        {
            connections_->erase(conn);
        }
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;
    typedef boost::shared_ptr<ConnectionList> ConnectionListPtr;

    void onStringMessage(const TcpConnectionPtr&,
                         const string& message,
                         Timestamp)
    {
        ConnectionListPtr connections = getConnectionList();;
        for (ConnectionList::iterator it = connections->begin();
                it != connections->end();
                ++it)
        {
            codec_.send(get_pointer(*it), message);
        }
    }

    ConnectionListPtr getConnectionList()
    {
        MutexLockGuard lock(mutex_);
        return connections_;
    }

    TcpServer server_;
    LengthHeaderCodec codec_;
    MutexLock mutex_;
    ConnectionListPtr connections_;
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
        if (argc > 2)
        {
            server.setThreadNum(atoi(argv[2]));
        }
        server.start();
        loop.loop();
    }
    else
    {
        printf("Usage: %s port [thread_num]\n", argv[0]);
    }
}

