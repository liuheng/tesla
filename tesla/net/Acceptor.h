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

#ifndef TESLA_NET_ACCEPTOR_H
#define TESLA_NET_ACCEPTOR_H

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/function.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/Channel.h>
#include <tesla/net/Socket.h>

namespace tesla
{
namespace net
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor
    : private tesla::base::Noncopyable
{
public:

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listening() const
    {
        return listening_;
    }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;
}; // class Acceptor

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_ACCEPTOR_H
