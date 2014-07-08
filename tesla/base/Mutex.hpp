/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/18/2014
 * Description
 *     mutex-based lock and scoped lock
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_MUTEX_HPP
#define TESLA_BASE_MUTEX_HPP

#include <tesla/base/Noncopyable.hpp>
#include <tesla/base/CurrentThread.h>

#include <assert.h>
#include <pthread.h>

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
__THROW __attribute__ ((__noreturn__));
__END_DECLS
#endif // NDEBUG

#define MCHECK(ret) ({ \
                        __typeof__ (ret) errnum = (ret); \
                        if (__builtin_expect(errnum != 0, 0)) | \
                            __assert_perror_fail (errnum, __FILE__, __LINE__, __func__); \
                    })

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ \
                        __typeof__ (ret) errnum = (ret); \
                        assert(errnum == 0); \
                        (void) errnum; \
                    })

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace tesla
{

namespace base
{

// Use as a data member of a class
class MutexLock
    : private Noncopyable
{
public:
    MutexLock()
        : holder_(0)
    {
        MCHECK(pthread_mutex_init(&mutex_, NULL));
    }

    ~MutexLock()
    {
        assert(holder_ == 0);
        MCHECK(pthread_mutex_destroy(&mutex_));
    }

    // must be called when locked, i.e. for assertion
    bool isLockedByThisThread() const
    {
        return holder_ == CurrentThread::tid();
    }

    void assertLocked() const
    {
        assert(isLockedByThisThread());
    }

    // internal usage
    void lock()
    {
        MCHECK(pthread_mutex_lock(&mutex_));
        assignHolder();
    }

    void unlock()
    {
        unassignHolder();
        MCHECK(pthread_mutex_unlock(&mutex_));
    }

    pthread_mutex_t* getPthreadMutex() /* non-const */
    {
        return &mutex_;
    }

private:
    friend class Condition;

    class UnassignGuard
        : private Noncopyable
    {
    public:
        UnassignGuard(MutexLock& owner)
            : owner_(owner)
        {
            owner_.unassignHolder();
        }

        ~UnassignGuard()
        {
            owner_.assignHolder();
        }

    private:
        MutexLock& owner_;
    };

    void unassignHolder()
    {
        holder_ = 0;
    }

    void assignHolder()
    {
        holder_ = CurrentThread::tid();
    }

    pthread_mutex_t mutex_;
    pid_t holder_;
}; // class MutexLock

// Use as a stack variable, eg.
class MutexLockGuard
    : private Noncopyable
{
public:
    explicit MutexLockGuard(MutexLock& mutex)
        : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~MutexLockGuard()
    {
        mutex_.unlock();
    }

private:

    MutexLock& mutex_;
}; // class MutexLockGuard

} // namespace base

} // namespace tesla

// Prevent misuse by temp-obj like:
// MutexLockGuard(mutex_);
#define MutexLockGuard(x) error "Missing guard object name"

#endif  // TESLA_BASE_MUTEX_HPP
