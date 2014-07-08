#include <tesla/net/Backend.h>
#include <tesla/net/backend/PollBackend.h>
#include <tesla/net/backend/EpollBackend.h>

#include <stdlib.h>

namespace tesla
{

namespace net
{

using namespace tesla::base;

Backend* Backend::newDefaultBackend(EventLoop* loop)
{
    if (::getenv("TESLA_USE_POLL"))
    {
        return new PollBackend(loop);
    }
    else
    {
        return new EpollBackend(loop);
    }
}

} // namespace net

} // namespace tesla
