/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_NET_TCPCLIENT_H
#define TESLA_NET_TCPCLIENT_H

#include <tesla/base/Noncopyable.hpp>
#include <tesla/base/Mutex.hpp>

#include <tesla/net/TcpConnection.h>

namespace tesla
{
namespace net
{

class Connector;

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
typedef std::shared_ptr<Connector> ConnectorPtr;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
typedef boost::shared_ptr<Connector> ConnectorPtr;
#endif // __GXX_EXPERIMENTAL_CXX0X__

class TcpClient
    : private tesla::base::Noncopyable
{
public:
    TcpClient(EventLoop* loop,
              const InetAddress& serverAddr,
              const tesla::base::string& name);
    ~TcpClient();  // force out-line dtor, for scoped_ptr members.

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        tesla::base::MutexLockGuard lock(mutex_);
        return connection_;
    }

    EventLoop* getLoop() const
    {
        return loop_;
    }
    bool retry() const;
    void enableRetry()
    {
        retry_ = true;
    }

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    void setConnectionCallback(ConnectionCallback&& cb)
    {
        connectionCallback_ = std::move(cb);
    }
    void setMessageCallback(MessageCallback&& cb)
    {
        messageCallback_ = std::move(cb);
    }
    void setWriteCompleteCallback(WriteCompleteCallback&& cb)
    {
        writeCompleteCallback_ = std::move(cb);
    }
#endif

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd);
    /// Not thread safe, but in loop
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_; // avoid revealing Connector
    const tesla::base::string name_;

    bool retry_;   // atmoic
    bool connect_; // atomic

    // always in loop thread
    int nextConnId_;
    mutable tesla::base::MutexLock mutex_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    TcpConnectionPtr connection_; // @GuardedBy mutex_
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_TCPCLIENT_H
