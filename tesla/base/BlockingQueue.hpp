/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *     BlockingQueue support
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_BLOCKINGQUEUE_HPP
#define TESLA_BASE_BLOCKINGQUEUE_HPP

#include <tesla/base/Condition.h>
#include <tesla/base/Mutex.hpp>

#include <tesla/base/Noncopyable.hpp>
#include <deque>
#include <assert.h>

namespace tesla
{

namespace base
{

template<typename T>
class BlockingQueue
    : private Noncopyable
{
public:
    BlockingQueue()
        : mutex_(),
          notEmpty_(mutex_),
          queue_()
    {
    }

    void put(const T& x)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify(); // wait morphing saves us
        // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
    }

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    void put(T&& x)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(x));
        notEmpty_.notify();
    }
    // FIXME: emplace()
#endif // __GXX_EXPERIMENTAL_CXX0X__

    T take()
    {
        MutexLockGuard lock(mutex_);
        // always use a while-loop, due to spurious wakeup
        while (queue_.empty())
        {
            notEmpty_.wait();
        }
        assert(!queue_.empty());
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
        T front(std::move(queue_.front()));
#else
        T front(queue_.front());
#endif // __GXX_EXPERIMENTAL_CXX0X__
        queue_.pop_front();
        return front;
    }

    size_t size() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

private:
    mutable MutexLock mutex_;
    Condition         notEmpty_;
    std::deque<T>     queue_;
}; // class BlockingQueue

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_BLOCKINGQUEUE_HPP
