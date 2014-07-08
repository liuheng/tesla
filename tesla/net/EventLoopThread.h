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

#ifndef TESLA_NET_EVENTLOOPTHREAD_H
#define TESLA_NET_EVENTLOOPTHREAD_H

#include <tesla/base/Condition.h>
#include <tesla/base/Mutex.hpp>
#include <tesla/base/Thread.h>
#include <tesla/base/Noncopyable.hpp>

namespace tesla
{
namespace net
{

class EventLoop;

class EventLoopThread
    : private tesla::base::Noncopyable
{
public:
    typedef boost::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    tesla::base::Thread thread_;
    tesla::base::MutexLock mutex_;
    tesla::base::Condition cond_;
    ThreadInitCallback callback_;
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_EVENTLOOPTHREAD_H
