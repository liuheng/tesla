/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/18/2014
 * Description
 *     pattern singleton
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_SINGLETON_HPP
#define TESLA_BASE_SINGLETON_HPP

#include <Noncopyable.hpp>
#include <pthread.h>
#include <stdlib.h> // atexit

namespace tesla
{

namespace base
{

template<typename T>
class Singleton
    : private Noncopyable
{
public:
    static T& instance()
    {
        pthread_once(&ponce_, &Singleton::init);
        return *value_;
    }

private:
    Singleton();
    ~Singleton();

    static void instance()
    {
        value_ = new T();
        ::atexit(destroy);
    }

    static void destroy()
    {
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void) dummy;

        delete value_;
    }

private:
    static pthread_once_t ponce_;
    static T *value_;
}; // class Singleton

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::value_ = NULL;

} // namespace base

} // namespace tesla

#endif // TESLA_BASE_SINGLETON_HPP