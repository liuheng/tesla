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

#ifndef TESLA_NET_CONNECTOR_H
#define TESLA_NET_CONNECTOR_H

#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/InetAddress.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

namespace tesla
{
namespace net
{

class Channel;
class EventLoop;

class Connector
    : private tesla::base::Noncopyable
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    , public std::enable_shared_from_this<Connector>
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
    , public boost::enable_shared_from_this<Connector>
#endif // __GXX_EXPERIMENTAL_CXX0X__
{
public:
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void (int sockfd)> NewConnectionCallback;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void (int sockfd)> NewConnectionCallback;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread

    const InetAddress& serverAddress() const
    {
        return serverAddr_;
    }

private:
    enum States { Disconnected, Connecting, Connected };
    static const int MAX_RETRY_DELAY_MS = 30*1000;
    static const int INIT_RETRY_DELAY_MS = 500;

    void setState(States s)
    {
        state_ = s;
    }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    void resetChannel();
    int removeAndResetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    States state_;  // FIXME: use atomic variable
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    std::scoped_ptr<Channel> channel_;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
    boost::scoped_ptr<Channel> channel_;
#endif // __GXX_EXPERIMENTAL_CXX0X__
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
    bool connect_; // atomic
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_CONNECTOR_H
