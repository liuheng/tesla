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

#ifndef TESLA_NET_TIMER_H
#define TESLA_NET_TIMER_H

#include <tesla/base/Noncopyable.hpp>
#include <tesla/base/Atomic.hpp>
#include <tesla/base/Timestamp.h>

#include <tesla/net/Callbacks.hpp>

namespace tesla
{

namespace net
{

///
/// Internal class for timer event.
///
class Timer
    : private tesla::base::Noncopyable
{
public:
    Timer(const TimerCallback& cb, tesla::base::Timestamp when, double interval)
        : callback_(cb),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(s_nCreated_.atomicIncrement())
    { }

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    Timer(TimerCallback&& cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(s_nCreated_.atomicIncrement())
    { }
#endif

    void run() const
    {
        callback_();
    }

    tesla::base::Timestamp expiration() const
    {
        return expiration_;
    }
    bool repeat() const
    {
        return repeat_;
    }
    int64_t sequence() const
    {
        return sequence_;
    }

    void restart(tesla::base::Timestamp now);

    static int64_t numCreated()
    {
        return s_nCreated_.atomicGet();
    }

private:
    const TimerCallback callback_;
    tesla::base::Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static tesla::base::AtomicInt64 s_nCreated_;
}; // class Timer

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_TIMER_H
