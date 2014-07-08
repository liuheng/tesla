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

#ifndef TESLA_NET_TCPSERVER_H
#define TESLA_NET_TCPSERVER_H

#include <tesla/base/Atomic.hpp>
#include <tesla/base/Types.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/TcpConnection.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <map>

namespace tesla
{
namespace net
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class TcpServer
    : private tesla::base::Noncopyable
{
public:
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void(EventLoop*)> ThreadInitCallback;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    enum Option
    {
        NoReusePort,
        ReusePort,
    };

    //TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const tesla::base::string& nameArg,
              Option option = NoReusePort);
    ~TcpServer();  // force out-line dtor, for scoped_ptr members.

    const tesla::base::string& hostport() const
    {
        return hostport_;
    }
    const tesla::base::string& name() const
    {
        return name_;
    }
    EventLoop* getLoop() const
    {
        return loop_;
    }

    /// Set the number of threads for handling input.
    ///
    /// Always accepts new connection in loop's thread.
    /// Must be called before call start
    /// @param numThreads
    /// - 0 means all I/O in loop's thread, no thread will created.
    ///   this is the default value.
    /// - 1 means all I/O in another thread.
    /// - N means a thread pool with N threads, new connections
    ///   are assigned on a round-robin basis.
    void setThreadNum(int numThreads);

    /// valid after calling start()
    EventLoopThreadPool* evThreadPool()
    {
        return get_pointer(evThreadpool_);
    }

    /// Starts the server if it's not listenning.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void start();

    /// Set thread-inited callback.
    /// Not thread safe, but in loop
    /// when all thread inited, invoke the user-callback
    void setThreadInitCallback(const ThreadInitCallback& cb)
    {
        threadInitCallback_ = cb;
    }
    /// Set connection callback.
    /// Not thread safe, but in loop
    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }
    /// Set message callback.
    /// Not thread safe, but in loop
    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }
    /// Set write complete callback.
    /// Not thread safe, but in loop
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

private:
    /// connections keyed by name
    typedef std::map<tesla::base::string, TcpConnectionPtr> ConnectionMap;

    /// Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);
    /// Thread safe
    void removeConnection(const TcpConnectionPtr& conn);
    /// Not thread safe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    /// event loop
    EventLoop* loop_;  // the acceptor loop, if numThread > 1

    const tesla::base::string hostport_;
    const tesla::base::string name_;

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    std::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
    std::scoped_ptr<EventLoopThreadPool> evThreadpool_;
#else // __GXX_EXPERIMENTAL_CXX0X__
    boost::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
    boost::scoped_ptr<EventLoopThreadPool> evThreadpool_;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    /// callbacks (for TcpConnections to use, managed here)
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    /// whether start is called, it's construct-initialized as 0
    tesla::base::AtomicInt32 started_;

    /// mark the count of the current connection
    int nextConnId_;

    /// connections keyed by name
    ConnectionMap connections_;
}; // class TcpServer

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_TCPSERVER_H
