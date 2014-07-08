#include <tesla/base/Types.hpp>
#include <tesla/base/Logger.h>

#include <tesla/net/SockOps.h>
#include <tesla/net/Endian.hpp>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <strings.h>  // bzero
#include <sys/socket.h>
#include <unistd.h>

namespace tesla
{

namespace net
{

using namespace tesla::base;

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const SA*>(implicit_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<SA*>(implicit_cast<void*>(addr));
}

#if VALGRIND
void setNonBlockAndCloseOnExec(int sockfd)
{
    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    // FIXME check

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
    // FIXME check

    (void)ret;
}
#endif

int SockOps::createNonblockingOrDie()
{
#if VALGRIND
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "createNonblockingOrDie failed";
    }

    setNonBlockAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "createNonblockingOrDie failed";
    }
#endif
    return sockfd;
}

void SockOps::bindOrDie(int sockfd, const struct sockaddr_in& addr)
{
    int ret = ::bind(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
    if (ret < 0)
    {
        LOG_SYSFATAL << "bindOrDie failed";
    }
}

void SockOps::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        LOG_SYSFATAL << "listenOrDie failed";
    }
}

int SockOps::accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
#if VALGRIND
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (savedErrno)
        {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            LOG_FATAL << "unexpected error of ::accept " << savedErrno;
            break;
        default:
            LOG_FATAL << "unknown error of ::accept " << savedErrno;
            break;
        }
    }
    return connfd;
}

int SockOps::connect(int sockfd, const struct sockaddr_in& addr)
{
    return ::connect(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
}

ssize_t SockOps::read(int sockfd, void *buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t SockOps::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t SockOps::write(int sockfd, const void *buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

void SockOps::close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        LOG_SYSERR << "close";
    }
}

void SockOps::shutdownWrite(int sockfd)
{
    if (::shutdown(sockfd, SHUT_WR) < 0)
    {
        LOG_SYSERR << "internalShutdownWrite";
    }
}

void SockOps::formatIpPort(char* buf, size_t size,
                           const struct sockaddr_in& addr)
{
    assert(size >= INET_ADDRSTRLEN);
    ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
    size_t end = ::strlen(buf);
    uint16_t port = networkToHost16(addr.sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port);
}

void SockOps::formatIp(char* buf, size_t size,
                       const struct sockaddr_in& addr)
{
    assert(size >= INET_ADDRSTRLEN);
    ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
}

void SockOps::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG_SYSERR << "fromIpPort";
    }
}

int SockOps::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

struct sockaddr_in SockOps::getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
    {
        LOG_SYSERR << "getLocalAddr";
    }
    return localaddr;
}

struct sockaddr_in SockOps::getPeerAddr(int sockfd)
{
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
    {
        LOG_SYSERR << "getPeerAddr";
    }
    return peeraddr;
}

bool SockOps::selfConnect(int sockfd)
{
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);
    return localaddr.sin_port == peeraddr.sin_port
           && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

} // namespace net

} // namespace tesla
