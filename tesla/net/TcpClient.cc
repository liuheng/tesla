#include <tesla/base/Logger.h>

#include <tesla/net/TcpClient.h>
#include <tesla/net/Connector.h>
#include <tesla/net/EventLoop.h>
#include <tesla/net/SockOps.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
typedef std::shared_ptr<Connector> ConnectorPtr;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/bind.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <stdio.h>  // snprintf

namespace tesla
{

namespace net
{

using namespace tesla::base;

void internalRemoveConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{
    //connector->
}

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr,
                     const string& name)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop, serverAddr)),
      name_(name),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      nextConnId_(1)
{
    connector_->setNewConnectionCallback(
        boost::bind(&TcpClient::newConnection, this, _1));
    // FIXME setConnectFailedCallback
    LOG_INFO << "TcpClient::TcpClient[" << name_
             << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient()
{
    LOG_INFO << "TcpClient::~TcpClient[" << name_
             << "] - connector " << get_pointer(connector_);
    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(mutex_);
        conn = connection_;
    }
    if (conn)
    {
        // FIXME: not 100% safe, if we are in different thread
        CloseCallback cb = boost::bind(internalRemoveConnection, loop_, _1);
        loop_->runInLoop(
            boost::bind(&TcpConnection::setCloseCallback, conn, cb));
    }
    else
    {
        connector_->stop();
        // FIXME: HACK
        loop_->runAfter(1, boost::bind(removeConnector, connector_));
    }
}

void TcpClient::connect()
{
    // FIXME: check state
    LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
             << connector_->serverAddress().toIpPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;

    {
        MutexLockGuard lock(mutex_);
        if (connection_)
        {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(SockOps::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;

    InetAddress localAddr(SockOps::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TcpConnectionPtr conn(new TcpConnection(loop_,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        boost::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_)
    {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                 << connector_->serverAddress().toIpPort();
        connector_->restart();
    }
}

} // namespace net

} // namespace tesla
