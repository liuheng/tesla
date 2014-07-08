/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *     BoundedBlockingQueue support
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_BOUNDEDBLOCKINGQUEUE_HPP
#define TESLA_BASE_BOUNDEDBLOCKINGQUEUE_HPP

#include <tesla/base/Condition.h>
#include <tesla/base/Mutex.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <boost/circular_buffer.hpp>

#include <assert.h>

namespace tesla
{

namespace base
{

template<typename T>
class BoundedBlockingQueue
    : private Noncopyable
{
public:
    explicit BoundedBlockingQueue(int maxSize)
        : mutex_(),
          notEmpty_(mutex_),
          notFull_(mutex_),
          queue_(maxSize)
    {
    }

    void put(const T& x)
    {
        MutexLockGuard lock(mutex_);
        while (queue_.full())
        {
            notFull_.wait();
        }
        assert(!queue_.full());
        queue_.push_back(x);
        notEmpty_.notify();
    }

    T take()
    {
        MutexLockGuard lock(mutex_);
        while (queue_.empty())
        {
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(queue_.front());
        queue_.pop_front();
        notFull_.notify();
        return front;
    }

    bool empty() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    bool full() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.full();
    }

    size_t size() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    size_t capacity() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.capacity();
    }

private:
    mutable MutexLock          mutex_;
    Condition                  notEmpty_;
    Condition                  notFull_;
    boost::circular_buffer<T>  queue_;
}; // class BoundedBlockQueue

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_BOUNDEDBLOCKINGQUEUE_HPP
