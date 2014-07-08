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

#ifndef TESLA_NET_TIMERID_HPP
#define TESLA_NET_TIMERID_HPP

#include <tesla/base/Copyable.hpp>

namespace tesla
{
namespace net
{

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId
    : public tesla::base::Copyable
{
public:
    TimerId()
        : timer_(NULL),
          sequence_(0)
    {
    }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer),
          sequence_(seq)
    {
    }

    // default copy-ctor, dtor and assignment are okay

    friend class TimerQueue;

private:
    Timer* timer_;
    int64_t sequence_;
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_TIMERID_HPP
