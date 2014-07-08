/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/18/2014
 * Description
 *     CurrentThread cache
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_CURRENTTHREAD_H
#define TESLA_BASE_CURRENTTHREAD_H

#include <sys/types.h>

namespace tesla
{

namespace base
{

class CurrentThread
{
public:
    static void cacheTid();

    static int tid();

    static const char* tidString();

    static const char* name();

    static bool isMainThread();

    static void sleepUsec(int64_t usec);

}; // class CurrentThread

} // namespace base

} // namespace tesla

#endif // TESLA_BASE_CURRENTTHREAD_H
