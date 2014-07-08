#include <tesla/base/Logger.h>
#include <tesla/base/Mutex.hpp>

#include <tesla/net/EventLoop.h>
#include <tesla/net/Channel.h>
#include <tesla/net/Backend.h>
#include <tesla/net/SockOps.h>
#include <tesla/net/TimerQueue.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <functional>
#else // __GXX_EXPERIMENTAL_CXX0X__
#include <boost/bind.hpp>
#endif // __GXX_EXPERIMENTAL_CXX0X__

#include <signal.h>
#include <sys/eventfd.h>

namespace tesla
{

namespace net
{

using namespace tesla::base;

/// this is stored in tls
__thread EventLoop* t_loopInThisThread = 0;

const int POLL_TIME_MS = 10000;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSERR << "create eventfd failed";
        abort();
    }
    return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
        // LOG_TRACE << "Ignore SIGPIPE";
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : threadId_(CurrentThread::tid())
    , looping_(false)
    , quit_(false)
    , eventHandling_(false)
    , callingPendingFunctors_(false)
    , iteration_(0)
    , currentActiveChannel_(NULL)
    , backend_(Backend::newDefaultBackend(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
    , timerQueue_(new TimerQueue(this))
{
    LOG_DEBUG << "EventLoop " << this << "created in thread " << threadId_;

    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(
        boost::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << CurrentThread::tid();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = backend_->run(POLL_TIME_MS, &activeChannels_);
        ++iteration_;
        if (Logger::getLogLevel() <= Logger::TRACE)
        {
            printActiveChannels();
        }
        // TODO sort channel by priority
        eventHandling_ = true;
        for (ChannelList::iterator it = activeChannels_.begin();
                it != activeChannels_.end(); ++it)
        {
            currentActiveChannel_ = *it;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        eventHandling_ = false;
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exists,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
// FIXME: remove duplication
void EventLoop::runInLoop(Functor&& cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor&& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));  // emplace_back
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

TimerId EventLoop::runAt(const Timestamp& time, TimerCallback&& cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback&& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback&& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}
#endif

void EventLoop::cancel(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();

    backend_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
               std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    backend_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = SockOps::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = SockOps::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
    for (ChannelList::const_iterator it = activeChannels_.begin();
            it != activeChannels_.end(); ++it)
    {
        const Channel* ch = *it;
        LOG_TRACE << "{" << ch->reventsToString() << "} ";
    }
}

} // namespace net

} // namespace tesla
