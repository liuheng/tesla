/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/18/2014
 * Description
 *     Thread support
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_THREADPOOL_H
#define TESLA_BASE_THREADPOOL_H

#include <tesla/base/Condition.h>
#include <tesla/base/Mutex.hpp>
#include <tesla/base/Thread.h>
#include <tesla/base/Types.hpp>
#include <tesla/base/Noncopyable.hpp>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/function.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <boost/ptr_container/ptr_vector.hpp>
#include <deque>

namespace tesla
{

namespace base
{

class ThreadPool
    : private Noncopyable
{
public:
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void ()> Task;
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void ()> Task;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    explicit ThreadPool(const string& name = string("ThreadPool"));
    ~ThreadPool();

    // Must be called before start().
    void setMaxQueueSize(int maxSize)
    {
        maxQueueSize_ = maxSize;
    }

    void start(int numThreads);
    void stop();

    // Could block if maxQueueSize > 0
    void run(const Task& f);
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    void run(Task&& f);
#endif // __GXX_EXPERIMENTAL_CXX0X__

private:
    bool isFull() const;
    void runInThread();
    Task take();

    MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;
    string name_;
    boost::ptr_vector<Thread> threads_;
    std::deque<Task> queue_;
    size_t maxQueueSize_;
    bool running_;
}; // class Threadpool

} // namespace base

} // namespace tesla

#endif // TESLA_BASE_THREADPOOL_H
