#include <tesla/base/Timestamp.h>

#include <sys/time.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS // for PRId64
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

#include <boost/static_assert.hpp>

namespace tesla
{

namespace base
{

BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));

Timestamp::Timestamp(int64_t microseconds)
    : microsecondsSinceEpoch_(microseconds)
{
}

string Timestamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = microsecondsSinceEpoch_ / MICROSECONDS_PER_SECOND;
    int64_t microseconds = microsecondsSinceEpoch_ % MICROSECONDS_PER_SECOND;
    //PRId64 is equal to "ld"
    snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

string Timestamp::toFormattedString(bool showMicroseconds) const
{
    char buf[32] = {0};
    time_t seconds = static_cast<time_t>(microsecondsSinceEpoch_ / MICROSECONDS_PER_SECOND);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroseconds)
    {
        int microseconds = static_cast<int>(microsecondsSinceEpoch_ % MICROSECONDS_PER_SECOND);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                 microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * MICROSECONDS_PER_SECOND + tv.tv_usec);
}

Timestamp Timestamp::invalid()
{
    return Timestamp();
}

} // namespace base

} // namespace tesla
