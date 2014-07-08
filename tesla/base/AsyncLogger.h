/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *     asynchronic logger, based on queues
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_ASYNCLOGGING_H
#define TESLA_BASE_ASYNCLOGGING_H

#include <tesla/base/BlockingQueue.hpp>
#include <tesla/base/BoundedBlockingQueue.hpp>
#include <tesla/base/CountdownLatch.h>
#include <tesla/base/Mutex.hpp>
#include <tesla/base/Thread.h>
#include <tesla/base/LogStream.h>
#include <tesla/base/Noncopyable.hpp>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#else
#include <boost/bind.hpp>
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace tesla
{

namespace base
{

class AsyncLogger
    : private Noncopyable
{
public:

    AsyncLogger(const string& basename,
                size_t rollSize,
                int flushInterval = 3);

    ~AsyncLogger()
    {
        if (running_)
        {
            stop();
        }
    }

    void append(const char* logline, int len);

    void start()
    {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop()
    {
        running_ = false;
        cond_.notify();
        thread_.join();
    }

private:

    // declare but not define, prevent compiler-synthesized functions
    AsyncLogger(const AsyncLogger&);  // ptr_container
    void operator=(const AsyncLogger&);  // ptr_container

    void threadFunc();

    typedef FixedBuffer<LARGE_BUFFER> Buffer;
    typedef boost::ptr_vector<Buffer> BufferVector;
    typedef BufferVector::auto_type BufferPtr;

    const int flushInterval_;
    bool running_;
    string basename_;
    size_t rollSize_;
    Thread thread_;
    CountdownLatch latch_;
    MutexLock mutex_;
    Condition cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
}; // class AsyncLogger

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_ASYNCLOGGING_H
