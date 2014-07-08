/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *     a count-down timer
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_COUNTDOWNLATCH_H
#define TESLA_BASE_COUNTDOWNLATCH_H

#include <tesla/base/Mutex.hpp>
#include <tesla/base/Condition.h>

#include <tesla/base/Noncopyable.hpp>

namespace tesla
{

namespace base
{

class CountdownLatch
    : private Noncopyable
{
public:

    explicit CountdownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
}; // class CountdownLatch

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_COUNTDOWNLATCH_H
