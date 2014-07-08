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

#ifndef TESLA_NET_TIMERQUEUE_H
#define TESLA_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include <tesla/base/Noncopyable.hpp>
#include <tesla/base/Mutex.hpp>
#include <tesla/base/Timestamp.h>

#include <tesla/net/Callbacks.hpp>
#include <tesla/net/Channel.h>

namespace tesla
{

namespace net
{

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue
    : private tesla::base::Noncopyable
{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    TimerId addTimer(const TimerCallback& cb,
                     tesla::base::Timestamp when,
                     double interval);
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    TimerId addTimer(TimerCallback&& cb,
                     tesla::base::Timestamp when,
                     double interval);
#endif

    void cancel(TimerId timerId);

private:

    // FIXME: use unique_ptr<Timer> instead of raw pointers.
    typedef std::pair<tesla::base::Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<Entry> getExpired(tesla::base::Timestamp now);
    void reset(const std::vector<Entry>& expired, tesla::base::Timestamp now);

    bool insert(Timer* timer);

    EventLoop* loop_;

    /// timer eventfd and it's channel
    const int timerfd_;
    Channel timerfdChannel_;

    // Timer list sorted by expiration
    TimerList timers_;

    /// timer sets
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;

    bool callingExpiredTimers_; /* atomic */
}; // class TimerQueue

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_TIMERQUEUE_H
