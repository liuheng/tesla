/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     time support, consider timezone
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_TIME_H
#define TESLA_BASE_TIME_H

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#else
#include <boost/shared_ptr.hpp>
#endif

#include <tesla/base/Copyable.hpp>
#include <time.h>

namespace tesla
{

namespace base
{

// Time for 1970~2030
class Time
    : public Copyable
{
public:
    explicit Time(const char* zonefile);
    Time(int eastOfUtc, const char* tzname);  // time with fixed timezone
    Time() {}  // an invalid time

    // default copy ctor/assignment/dtor are Okay.

    bool valid() const
    {
        // 'explicit operator bool() const' in C++11
        return static_cast<bool>(data_);
    }

    // time conversion functions
    struct tm toLocalTime(time_t secondsSinceEpoch) const;
    // gmtime(3)
    static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);

    time_t fromLocalTime(const struct tm&) const;
    // timegm(3)
    static time_t fromUtcTime(const struct tm&);
    // year in [1900..2500], month in [1..12], day in [1..31]
    static time_t fromUtcTime(int year, int month, int day,
                              int hour, int minute, int seconds);

    struct Data;

private:

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
    std::shared_ptr<Data> data_;
#else
    boost::shared_ptr<Data> data_;
#endif

}; // class Time

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_TIME_H
