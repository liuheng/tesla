/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     Timestamp in UTC, in microsecond
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_TIMESTAMP_H
#define TESLA_BASE_TIMESTAMP_H

#include <tesla/base/Copyable.hpp>
#include <tesla/base/Types.hpp>

namespace tesla
{

namespace base
{

//
// This class is immutable.
// It's recommended to pass it by value, since it's passed in register on x64.
//
class Timestamp
    : public Copyable
{
public:
    // constructors
    Timestamp()
        : microsecondsSinceEpoch_(0)
    {
    }

    explicit Timestamp(int64_t microseconds);

    void swap(Timestamp& that)
    {
        std::swap(microsecondsSinceEpoch_, that.microsecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    string toString() const;
    string toFormattedString(bool showMicroseconds = true) const;

    inline bool valid() const
    {
        return microsecondsSinceEpoch_ > 0;
    }

    // for internal usage.
    inline int64_t microSecondsSinceEpoch() const
    {
        return microsecondsSinceEpoch_;
    }
    inline time_t secondsSinceEpoch() const
    {
        return static_cast<time_t>(microsecondsSinceEpoch_ / MICROSECONDS_PER_SECOND);
    }

    // Get time of now.
    static Timestamp now();
    static Timestamp invalid();

    const static int MICROSECONDS_PER_SECOND = 1000 * 1000;

private:
    int64_t microsecondsSinceEpoch_;
}; // class Timestamp

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator>(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
}

inline bool operator<=(Timestamp lhs, Timestamp rhs)
{
    return !(lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch());
}

inline bool operator>=(Timestamp lhs, Timestamp rhs)
{
    return !(lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch());
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator!=(Timestamp lhs, Timestamp rhs)
{
    return !(lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch());
}

// Gets time difference of two timestamps, result in microsecond
inline int64_t timespanInMicrosecond(Timestamp high, Timestamp low)
{
    return high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
}

// Gets time difference of two timestamps, result in seconds.
inline double timespanInSecond(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::MICROSECONDS_PER_SECOND;
}

// Add microseconds to given timestamp.
inline Timestamp addTime(Timestamp timestamp, int64_t microseconds)
{
    return Timestamp(timestamp.microSecondsSinceEpoch() + microseconds);
}

// Add seconds to given timestamp.
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::MICROSECONDS_PER_SECOND);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

} // namespace base

} // namespace tesla

#endif  // TESLA_BASE_TIMESTAMP_H
