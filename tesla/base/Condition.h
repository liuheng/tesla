/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     Condition variable
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_CONDITION_H
#define TESLA_BASE_CONDITION_H

#include <tesla/base/Mutex.hpp>
#include <tesla/base/Noncopyable.hpp>

#include <pthread.h>

namespace tesla
{

namespace base
{

class Condition
    : private Noncopyable
{
public:
    explicit Condition(MutexLock& mutex)
        : mutex_(mutex)
    {
        MCHECK(pthread_cond_init(&pcond_, NULL));
    }

    ~Condition()
    {
        MCHECK(pthread_cond_destroy(&pcond_));
    }

    void wait()
    {
        MutexLock::UnassignGuard ug(mutex_);
        MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
    }

    // returns true if time out, false otherwise.
    bool waitForSeconds(int seconds);

    void notify()
    {
        MCHECK(pthread_cond_signal(&pcond_));
    }

    void notifyAll()
    {
        MCHECK(pthread_cond_broadcast(&pcond_));
    }

private:
    MutexLock& mutex_;
    pthread_cond_t pcond_;
}; // class Condition

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_CONDITION_H
