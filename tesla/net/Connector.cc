#include <tesla/base/Logger.h>

#include <tesla/net/Connector.h>
#include <tesla/net/Channel.h>
#include <tesla/net/EventLoop.h>
#include <tesla/net/SockOps.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/bind.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <errno.h>

namespace tesla
{

namespace net
{

using namespace tesla::base;

const int Connector::MAX_RETRY_DELAY_MS;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(Disconnected),
      retryDelayMs_(INIT_RETRY_DELAY_MS)
{
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
    LOG_DEBUG << "dtor[" << this << "]";
    assert(!channel_);
}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(boost::bind(&Connector::startInLoop, this)); // FIXME: unsafe
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == Disconnected);
    if (connect_)
    {
        connect();
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(boost::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
    // FIXME: cancel timer
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == Connecting)
    {
        setState(Disconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = SockOps::createNonblockingOrDie();
    int ret = SockOps::connect(sockfd, serverAddr_.getSockAddrInet());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
        close(sockfd);
        break;

    default:
        LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
        close(sockfd);
        // connectErrorCallback_();
        break;
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(Disconnected);
    retryDelayMs_ = INIT_RETRY_DELAY_MS;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd)
{
    setState(Connecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(
        boost::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    channel_->setErrorCallback(
        boost::bind(&Connector::handleError, this)); // FIXME: unsafe

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(boost::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::handleWrite()
{
    LOG_TRACE << "Connector::handleWrite " << state_;

    if (state_ == Connecting)
    {
        int sockfd = removeAndResetChannel();
        int err = SockOps::getSocketError(sockfd);
        if (err)
        {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                     << err << " " << strerror_tl(err);
            retry(sockfd);
        }
        else if (SockOps::selfConnect(sockfd))
        {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        }
        else
        {
            setState(Connected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                close(sockfd);
            }
        }
    }
    else
    {
        // what happened?
        assert(state_ == Disconnected);
    }
}

void Connector::handleError()
{
    LOG_ERROR << "Connector::handleError state=" << state_;
    if (state_ == Connecting)
    {
        int sockfd = removeAndResetChannel();
        int err = SockOps::getSocketError(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    close(sockfd);
    setState(Disconnected);
    if (connect_)
    {
        LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
                 << " in " << retryDelayMs_ << " milliseconds. ";
        loop_->runAfter(retryDelayMs_/1000.0,
                        boost::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, MAX_RETRY_DELAY_MS);
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

} // namespace net

} // namespace tesla
