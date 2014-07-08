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

#ifndef TESLA_NET_EVENTLOOPTHREADPOOL_H
#define TESLA_NET_EVENTLOOPTHREADPOOL_H

#include <vector>
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/function.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <tesla/base/Noncopyable.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

namespace tesla
{

namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
    : private tesla::base::Noncopyable
{
public:
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
#include <memory>
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void(EventLoop*)> ThreadInitCallback;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads)
    {
        nThreads_ = numThreads;
    }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    // valid after calling start()
    EventLoop* getNextLoop();
    // valid after calling start()
    std::vector<EventLoop*> getAllLoops();

private:

    EventLoop* baseLoop_;
    bool started_;
    int nThreads_;
    int next_;
    boost::ptr_vector<EventLoopThread> threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_EVENTLOOPTHREADPOOL_H
