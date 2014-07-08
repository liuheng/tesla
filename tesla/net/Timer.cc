#include <tesla/net/Timer.h>

namespace tesla
{

namespace net
{

using namespace tesla::base;

AtomicInt64 Timer::s_nCreated_;

void Timer::restart(Timestamp now)
{
    if (repeat_)
    {
        expiration_ = addTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}

} // namesapce net

} // namespace tesla
