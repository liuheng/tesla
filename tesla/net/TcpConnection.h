/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *     TcpConnection class, mainly to envelop socket-channel pairs and callbacks
 *     Explose socket operations, such as send, recv, close
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_NET_TCPCONNECTION_H
#define TESLA_NET_TCPCONNECTION_H

#include <tesla/base/StringPiece.hpp>
#include <tesla/base/Types.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/Callbacks.hpp>
#include <tesla/net/Buffer.h>
#include <tesla/net/InetAddress.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
typedef std::shared_ptr<Connector> ConnectorPtr;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <boost/any.hpp>

namespace tesla
{
namespace net
{

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection
///
/// Should NOT be used directly
class TcpConnection
    : private tesla::base::Noncopyable
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    , public std::enable_shared_from_this<TcpConnection>
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
    , public boost::enable_shared_from_this<TcpConnection>
#endif // __GXX_EXPERIMENTAL_CXX0X__
{
public:
    /// Constructs a TcpConnection with a connected sockfd
    TcpConnection(EventLoop* loop,
                  const tesla::base::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const
    {
        return loop_;
    }
    const tesla::base::string& name() const
    {
        return name_;
    }
    bool connected() const
    {
        return state_ == Connected;
    }

    /// Get local/peer address
    const InetAddress& localAddress() const
    {
        return localAddr_;
    }
    const InetAddress& peerAddress() const
    {
        return peerAddr_;
    }

    ///
    ///socket operations
    ///
    // void send(tesla::base::string&& message); // C++11
    void send(const void* message, int len);
    void send(const tesla::base::StringPiece& message);
    // void send(Buffer&& message); // C++11
    void send(Buffer* message);  // this one will swap data
    void shutdown(); // NOT thread safe, no simultaneous calling
    // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);

    ///
    /// get/set Context
    ///
    void setContext(const boost::any& context)
    {
        context_ = context;
    }
    const boost::any& getContext() const
    {
        return context_;
    }
    boost::any* getMutableContext()
    {
        return &context_;
    }

    ///
    /// set Callbacks
    ///
    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }
    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }

    ///
    /// Connection Est/Des callbacks
    ///
    /// called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    /// called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once

    /// Get I/O buffer
    Buffer* inputBuffer()
    {
        return &inputBuffer_;
    }
    Buffer* outputBuffer()
    {
        return &outputBuffer_;
    }

private:
    /// Connection state
    enum StateE
    {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    };

    /// Handlers
    void handleRead(tesla::base::Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    /// I/O operations in EventLoop thread
    // void sendInLoop(tesla::base::string&& message);
    void sendInLoop(const tesla::base::StringPiece& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    // void shutdownAndForceCloseInLoop(double seconds);
    void forceCloseInLoop();

    /// Connection state
    void setState(StateE s)
    {
        state_ = s;
    }
    StateE state_;  // FIXME: use atomic variable

    /// one TcpConnection is binded in one loop
    EventLoop* loop_;

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    std::scoped_ptr<Socket> socket_;
    std::scoped_ptr<Channel> channel_;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
    boost::scoped_ptr<Socket> socket_;
    boost::scoped_ptr<Channel> channel_;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    /// local and peer InetAddress
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    /// Callbacks
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    /// I/O Buffer
    Buffer inputBuffer_;
    Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.

    /// Watermark
    size_t highWaterMark_;

    /// Context
    boost::any context_;

    /// Connection Name
    const tesla::base::string name_;

    // FIXME: creationTime_, lastReceiveTime_
    //        bytesReceived_, bytesSent_
};

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
#endif // __GXX_EXPERIMENTAL_CXX0X__

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_TCPCONNECTION_H
