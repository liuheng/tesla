/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     Date in Gregorian calendar
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_DATE_H
#define TESLA_BASE_DATE_H

#include <tesla/base/Copyable.hpp>
#include <tesla/base/Types.hpp>
#include <time.h>

namespace tesla
{

namespace base
{

//
// This class is immutable.
// It's recommended to pass it by value, since it's passed in register on x64.
//
class Date
    : public Copyable
{
public:

    struct YearMonthDay
    {
        int year; // [1900..2500]
        int month;  // [1..12]
        int day;  // [1..31]
    };

    static const int DAYS_PER_WEEK = 7;
    static const int JULIAN_DAY_OF_1970_01_01;

    // constructs

    // Constucts an invalid Date.
    Date()
        : julianDayNumber_(0)
    {}

    // Constucts a yyyy-mm-dd Date.
    // 1 <= month <= 12
    Date(int year, int month, int day);

    // Constucts a Date from Julian Day Number.
    explicit Date(int julianDayNum)
        : julianDayNumber_(julianDayNum)
    {}

    // Constucts a Date from struct tm
    explicit Date(const struct tm&);

    // default copy/assignment/dtor are Okay

    void swap(Date& that)
    {
        std::swap(julianDayNumber_, that.julianDayNumber_);
    }

    inline bool valid() const
    {
        return julianDayNumber_ > 0;
    }

    // Converts to yyyy-mm-dd format.
    string toString() const;

    struct YearMonthDay yearMonthDay() const;

    int year() const
    {
        return yearMonthDay().year;
    }

    int month() const
    {
        return yearMonthDay().month;
    }

    int day() const
    {
        return yearMonthDay().day;
    }

    // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
    inline int weekDay() const
    {
        return (julianDayNumber_+1) % DAYS_PER_WEEK;
    }

    inline int julianDayNumber() const
    {
        return julianDayNumber_;
    }

private:
    int julianDayNumber_;
}; // class Date

inline bool operator<(Date x, Date y)
{
    return x.julianDayNumber() < y.julianDayNumber();
}

inline bool operator<=(Date x, Date y)
{
    return !(x.julianDayNumber() > y.julianDayNumber());
}

inline bool operator>(Date x, Date y)
{
    return x.julianDayNumber() > y.julianDayNumber();
}

inline bool operator>=(Date x, Date y)
{
    return !(x.julianDayNumber() < y.julianDayNumber());
}

inline bool operator==(Date x, Date y)
{
    return x.julianDayNumber() == y.julianDayNumber();
}

inline bool operator!=(Date x, Date y)
{
    return !(x.julianDayNumber() == y.julianDayNumber());
}

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_DATE_H
