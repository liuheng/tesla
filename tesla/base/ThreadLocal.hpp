/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/18/2014
 * Description
 *     Thread local storage support
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_THREADLOCAL_HPP
#define TESLA_BASE_THREADLOCAL_HPP

#include <tesla/base/Noncopyable.hpp>
#include <pthread.h>

namespace tesla
{

namespace base
{

template<typename T>
class ThreadLocal
    : private Noncopyable
{
public:
    ThreadLocal()
    {
        pthread_key_create(&pkey_, &ThreadLocal::destructor);
    }

    ~ThreadLocal()
    {
        pthread_key_delete(pkey_);
    }

    T& value()
    {
        T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));
        if (!perThreadValue)
        {
            T* newObj = new T();
            pthread_setspecific(pkey_, newObj);
            perThreadValue = newObj;
        }
        return *perThreadValue;
    }

private:

    static void destructor(void *x)
    {
        T* obj = static_cast<T*>(x);
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void) dummy;
        delete obj;
    }

private:
    pthread_key_t pkey_;
}; // class ThreadLocal

} // namespace base

} // namespace tesla

#endif // TESLA_BASE_THREADLOCAL_HPP
