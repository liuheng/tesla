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

#ifndef TESLA_NET_SOCKOPS_H
#define TESLA_NET_SOCKOPS_H

#include <arpa/inet.h>
#include <tesla/base/Noncopyable.hpp>

namespace tesla
{

namespace net
{

class SockOps
    : private tesla::base::Noncopyable
{
public:
    ///
    /// Creates a non-blocking socket file descriptor,
    /// abort if any error.
    static int createNonblockingOrDie();

    static int connect(int sockfd, const struct sockaddr_in& addr);
    static void bindOrDie(int sockfd, const struct sockaddr_in& addr);
    static void listenOrDie(int sockfd);
    static int accept(int sockfd, struct sockaddr_in* addr);
    static ssize_t read(int sockfd, void *buf, size_t count);
    static ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
    static ssize_t write(int sockfd, const void *buf, size_t count);
    static void close(int sockfd);
    static void shutdownWrite(int sockfd);
    static int getSocketError(int sockfd);
    static struct sockaddr_in getLocalAddr(int sockfd);
    static struct sockaddr_in getPeerAddr(int sockfd);
    static bool selfConnect(int sockfd);

    static void formatIpPort(char* buf, size_t size,
                             const struct sockaddr_in& addr);
    static void formatIp(char* buf, size_t size,
                         const struct sockaddr_in& addr);

    static void fromIpPort(const char* ip, uint16_t port,
                           struct sockaddr_in* addr);

}; // class SockOps

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_SOCKOPS_H
