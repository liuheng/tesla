/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *     Core reactor implementation
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_NET_EVENTLOOP_H
#define TESLA_NET_EVENTLOOP_H

#include <vector>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#include <functional>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <tesla/base/Mutex.hpp>
#include <tesla/base/CurrentThread.h>
#include <tesla/base/Timestamp.h>
#include <tesla/base/Noncopyable.hpp>

#include <tesla/net/Callbacks.hpp>
#include <tesla/net/TimerId.hpp>

namespace tesla
{
namespace net
{

class Channel;
class Backend;
class TimerQueue;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop
    : private tesla::base::Noncopyable
{
public:
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void()> Functor;
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void()> Functor;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    EventLoop();
    ~EventLoop();  // force out-line dtor, for scoped_ptr members.

    ///
    /// Loops forever.
    ///
    /// Must be called in the same thread as creation of the object.
    ///
    void loop();

    /// Quits loop.
    ///
    /// This is not 100% thread safe, if you call through a raw pointer,
    /// better to call through shared_ptr<EventLoop> for 100% safety.
    void quit();

    ///
    /// Time when poll returns, usually means data arrival.
    ///
    tesla::base::Timestamp pollReturnTime() const
    {
        return pollReturnTime_;
    }

    /// how many times do we call backend->poll
    int64_t iteration() const
    {
        return iteration_;
    }

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(const Functor& cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(const Functor& cb);

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    void runInLoop(Functor&& cb);
    void queueInLoop(Functor&& cb);
#endif // __GXX_EXPERIMENTAL_CXX0X__

    ///
    /// for timers
    ///
    /// Runs callback at 'time'.
    /// Safe to call from other threads.
    ///
    TimerId runAt(const tesla::base::Timestamp& time, const TimerCallback& cb);
    ///
    /// Runs callback after @c delay seconds.
    /// Safe to call from other threads.
    ///
    TimerId runAfter(double delay, const TimerCallback& cb);
    ///
    /// Runs callback every @c interval seconds.
    /// Safe to call from other threads.
    ///
    TimerId runEvery(double interval, const TimerCallback& cb);
    ///
    /// Cancels the timer.
    /// Safe to call from other threads.
    ///
    void cancel(TimerId timerId);

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    TimerId runAt(const tesla::base::Timestamp& time, TimerCallback&& cb);
    TimerId runAfter(double delay, TimerCallback&& cb);
    TimerId runEvery(double interval, TimerCallback&& cb);
#endif

    /// internal usage
    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    pid_t threadId() const { return threadId_; }
    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const
    {
        return threadId_ == tesla::base::CurrentThread::tid();
    }
    // bool callingPendingFunctors() const { return callingPendingFunctors_; }
    bool eventHandling() const
    {
        return eventHandling_;
    }

    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();
    void handleRead();  // waked up
    void doPendingFunctors();

    void printActiveChannels() const; // for debug use

    typedef std::vector<Channel*> ChannelList;

    const pid_t threadId_; // one loop maps one thread
    bool looping_; /* atomic */
    bool quit_; /* atomic and shared between threads, okay on x86, I guess. */
    bool eventHandling_; /* atomic */
    bool callingPendingFunctors_; /* atomic */
    int64_t iteration_;
    tesla::base::Timestamp pollReturnTime_;

    int wakeupFd_;

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    // unlike in TimerQueue, which is an internal class,
    // we don't expose Channel to client.
    std::scoped_ptr<Channel> wakeupChannel_;
    std::scoped_ptr<Backend> backend_;
    std::scoped_ptr<TimerQueue> timerQueue_;
#else // __GXX_EXPERIMENTAL_CXX0X__
    // unlike in TimerQueue, which is an internal class,
    // we don't expose Channel to client.
    boost::scoped_ptr<Channel> wakeupChannel_;
    boost::scoped_ptr<Backend> backend_;
    boost::scoped_ptr<TimerQueue> timerQueue_;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;
    tesla::base::MutexLock mutex_;
    std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_
}; // class EventLoop

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_EVENTLOOP_H
