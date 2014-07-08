#ifndef TESLA_EXAMPLES_ASIO_CHAT_CODEC_H
#define TESLA_EXAMPLES_ASIO_CHAT_CODEC_H

#include <tesla/base/Logger.h>
#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/Buffer.h>
#include <tesla/net/Endian.hpp>
#include <tesla/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

using tesla::base::Logger;

class LengthHeaderCodec
    : private tesla::base::Noncopyable
{
public:
    typedef boost::function<void (const tesla::net::TcpConnectionPtr&,
                                  const tesla::base::string& message,
                                  tesla::base::Timestamp)> StringMessageCallback;

    explicit LengthHeaderCodec(const StringMessageCallback& cb)
        : messageCallback_(cb)
    {
    }

    void onMessage(const tesla::net::TcpConnectionPtr& conn,
                   tesla::net::Buffer* buf,
                   tesla::base::Timestamp receiveTime)
    {
        while (buf->readableBytes() >= HEADER_LENGTH) // HEADER_LENGTH == 4
        {
            // FIXME: use Buffer::peekInt32()
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
            const int32_t len = tesla::net::networkToHost32(be32);
            if (len > 65536 || len < 0)
            {
                LOG_ERROR << "Invalid length " << len;
                conn->shutdown();  // FIXME: disable reading
                break;
            }
            else if (buf->readableBytes() >= len + HEADER_LENGTH)
            {
                buf->retrieve(HEADER_LENGTH);
                tesla::base::string message(buf->peek(), len);
                messageCallback_(conn, message, receiveTime);
                buf->retrieve(len);
            }
            else
            {
                break;
            }
        }
    }

    // FIXME: TcpConnectionPtr
    void send(tesla::net::TcpConnection* conn,
              const tesla::base::StringPiece& message)
    {
        tesla::net::Buffer buf;
        buf.append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = tesla::net::hostToNetwork32(len);
        buf.prepend(&be32, sizeof be32);
        conn->send(&buf);
    }

private:
    StringMessageCallback messageCallback_;
    const static size_t HEADER_LENGTH = sizeof(int32_t);
};

#endif  // TESLA_EXAMPLES_ASIO_CHAT_CODEC_H
