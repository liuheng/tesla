#include <tesla/net/EventLoopThreadpool.h>
#include <tesla/net/EventLoop.h>
#include <tesla/net/EventLoopThread.h>

#include <vector>
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/bind.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

namespace tesla
{

namespace net
{

using namespace tesla::base;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      nThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < nThreads_; ++i)
    {
        EventLoopThread* t = new EventLoopThread(cb);
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
    if (nThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (implicit_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}

} // namespace net

} // namespace tesla
