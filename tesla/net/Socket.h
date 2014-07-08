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

#ifndef TESLA_NET_SOCKET_H
#define TESLA_NET_SOCKET_H

#include <tesla/base/Noncopyable.hpp>

namespace tesla
{
///
/// TCP networking.
///
namespace net
{

class InetAddress;

///
/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delagated to OS.
class Socket
    : private tesla::base::Noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    { }

    // Socket(Socket&&) // move constructor in C++11
    ~Socket();

    int fd() const
    {
        return sockfd_;
    }

    /// abort if address in use
    void bindAddress(const InetAddress& localaddr);
    /// abort if address in use
    void listen();

    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    ///
    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    ///
    void setTcpNoDelay(bool on);

    ///
    /// Enable/disable SO_REUSEADDR
    ///
    void setReuseAddr(bool on);

    ///
    /// Enable/disable SO_REUSEPORT
    ///
    void setReusePort(bool on);

    ///
    /// Enable/disable SO_KEEPALIVE
    ///
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_SOCKET_H
