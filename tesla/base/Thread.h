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

#ifndef TESLA_BASE_THREAD_H
#define TESLA_BASE_THREAD_H

#include <tesla/base/Atomic.hpp>
#include <tesla/base/Types.hpp>
#include <tesla/base/Noncopyable.hpp>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#include <functional>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <pthread.h>

namespace tesla
{

namespace base
{

class Thread
    : private Noncopyable
{
public:
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    typedef std::function<void ()> ThreadFunc;
#else // __GXX_EXPERIMENTAL_CXX0X__
    typedef boost::function<void ()> ThreadFunc;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    explicit Thread(const ThreadFunc&, const string& name = string());
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    explicit Thread(ThreadFunc&&, const string& name = string());
#endif // __GXX_EXPERIMENTAL_CXX0X__
    ~Thread();

    void start();
    int join(); // return pthread_join()

    bool started() const
    {
        return started_;
    }
    // pthread_t pthreadId() const { return pthreadId_; }
    pid_t tid() const
    {
        return *tid_;
    }
    const string& name() const
    {
        return name_;
    }

    static int getThreadCount()
    {
        return nCreated_.atomicGet();
    }

private:
    void setDefaultName();

    pthread_t  pthreadId_;
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    std::shared_ptr<pid_t> tid_;
#else // __GXX_EXPERIMENTAL_CXX0X__
    boost::shared_ptr<pid_t> tid_;
#endif // __GXX_EXPERIMENTAL_CXX0X__

    ThreadFunc func_;
    string     name_;

    bool       started_;
    bool       joined_;

    static AtomicInt32 nCreated_;
};

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_THREAD_H
