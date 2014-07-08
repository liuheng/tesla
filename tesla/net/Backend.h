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

#ifndef TESLA_NET_BACKEND_H
#define TESLA_NET_BACKEND_H

#include <vector>
#include <tesla/base/Noncopyable.hpp>
#include <tesla/base/Timestamp.h>

#include <tesla/net/EventLoop.h>

namespace tesla
{
namespace net
{

class Channel;

///
/// Base class for IO multiplexing/demultiplexing
///
/// This class doesn't own the Channel objects.
class Backend
    : private tesla::base::Noncopyable
{
public:
    typedef std::vector<Channel*> ChannelList;

    Backend(EventLoop* loop)
        : ownerLoop_(loop)
    {}
    virtual ~Backend()
    {}

    /// Polls the I/O events.
    /// Must be called in the loop thread.
    virtual tesla::base::Timestamp run(int timeoutMs, ChannelList* activeChannels) = 0;

    /// Changes the interested I/O events.
    /// Must be called in the loop thread.
    virtual void updateChannel(Channel* channel) = 0;

    /// Remove the channel, when it destructs.
    /// Must be called in the loop thread.
    virtual void removeChannel(Channel* channel) = 0;

    static Backend* newDefaultBackend(EventLoop* loop);

    void assertInLoopThread()
    {
        ownerLoop_->assertInLoopThread();
    }

private:
    EventLoop* ownerLoop_;
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_BACKEND_H
