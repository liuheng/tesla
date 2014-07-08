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

#ifndef TESLA_NET_CHANNEL_H
#define TESLA_NET_CHANNEL_H

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#include <functional>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <tesla/base/Timestamp.h>
#include <tesla/base/Noncopyable.hpp>

namespace tesla
{
namespace net
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel
    : private tesla::base::Noncopyable
{
public:

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void()> EventCallback;
    typedef std::function<void(tesla::base::Timestamp)> ReadEventCallback;
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void()> EventCallback;
    typedef boost::function<void(tesla::base::Timestamp)> ReadEventCallback;
#endif // __GXX_EXPERIMENTAL_CXX0X__


    Channel(EventLoop* loop, int fd);
    ~Channel();

    ///
    /// event handler
    ///
    void handleEvent(tesla::base::Timestamp receiveTime);

    ///
    /// set callbacks
    ///
    void setReadCallback(const ReadEventCallback& cb)
    {
        readCallback_ = cb;
    }
    void setWriteCallback(const EventCallback& cb)
    {
        writeCallback_ = cb;
    }
    void setCloseCallback(const EventCallback& cb)
    {
        closeCallback_ = cb;
    }
    void setErrorCallback(const EventCallback& cb)
    {
        errorCallback_ = cb;
    }
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    void setReadCallback(ReadEventCallback&& cb)
    {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback&& cb)
    {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback&& cb)
    {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback&& cb)
    {
        errorCallback_ = std::move(cb);
    }
#endif

    /// Tie this channel to the owner object managed by shared_ptr,
    /// prevent the owner object being destroyed in handleEvent.
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    void tie(const std::shared_ptr<void>&);
#else // __GXX_EXPERIMENTAL_CXX0X__
    void tie(const boost::shared_ptr<void>&);
#endif // __GXX_EXPERIMENTAL_CXX0X__

    ///
    /// event management
    ///
    int events() const
    {
        return events_;
    }

    // int revents() const { return revents_; }
    void setRevents(int revt)
    {
        revents_ = revt;    // used by backend
    }
    bool isNoneEvent() const
    {
        return events_ == NONE_EVENT;
    }

    void enableReading()
    {
        events_ |= READ_EVENT;
        update();
    }
    // void disableReading() { events_ &= ~READ_EVENT; update(); }
    void enableWriting()
    {
        events_ |= WRITE_EVENT;
        update();
    }
    void disableWriting()
    {
        events_ &= ~WRITE_EVENT;
        update();
    }
    void disableAll()
    {
        events_ = NONE_EVENT;
        update();
    }
    bool isWriting() const
    {
        return events_ & WRITE_EVENT;
    }

    // for Poller
    int index()
    {
        return index_;
    }
    void setIndex(int idx)
    {
        index_ = idx;
    }

    // for debug
    tesla::base::string reventsToString() const;

    void doNotLogHup()
    {
        logHup_ = false;
    }

    int fd() const
    {
        return fd_;
    }

    EventLoop* ownerLoop()
    {
        return loop_;
    }

    void remove();

private:
    /// update fds
    void update();
    void handleEventWithGuard(tesla::base::Timestamp receiveTime);

    static const int NONE_EVENT;
    static const int READ_EVENT;
    static const int WRITE_EVENT;

    EventLoop* loop_;
    const int  fd_;

    /// what event(s) do we care about
    int        events_;

    /// received events
    int        revents_;

    int        index_; // used by backends

    /// debug POLLHUP
    bool       logHup_;

    /// shared_ptr object helper
    bool tied_;
    boost::weak_ptr<void> tie_;

    /// whether we are handling events now
    bool eventHandling_;

    /// callbacks
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
}; // class Channel

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_CHANNEL_H
