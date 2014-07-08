#include <tesla/base/Logger.h>

#include <tesla/net/backend/EpollBackend.h>
#include <tesla/net/Channel.h>

#include <boost/static_assert.hpp>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

namespace tesla
{

namespace net
{

using namespace tesla::base;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);

const int INDEX_NEW = -1;
const int INDEX_ADDED = 1;
const int INDEX_DELETED = 2;

EpollBackend::EpollBackend(EventLoop* loop)
    : Backend(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(INIT_EVENTLIST_SIZE)
{
    if (epollfd_ < 0)
    {
        LOG_SYSFATAL << "EpollBackend::EpollBackend";
    }
}

EpollBackend::~EpollBackend()
{
    ::close(epollfd_);
}

Timestamp EpollBackend::run(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
        if (implicit_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_TRACE << " nothing happended";
    }
    else
    {
        // error happens, log uncommon ones
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_SYSERR << "EpollBackend::poll()";
        }
    }
    return now;
}

void EpollBackend::fillActiveChannels(int numEvents,
                                      ChannelList* activeChannels) const
{
    assert(implicit_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollBackend::updateChannel(Channel* channel)
{
    Backend::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    const int index = channel->index();
    if (index == INDEX_NEW || index == INDEX_DELETED)
    {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == INDEX_NEW)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else // index == INDEX_DELETED
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setIndex(INDEX_ADDED);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == INDEX_ADDED);
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(INDEX_DELETED);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollBackend::removeChannel(Channel* channel)
{
    Backend::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == INDEX_ADDED || index == INDEX_DELETED);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == INDEX_ADDED)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(INDEX_NEW);
}

void EpollBackend::update(int operation, Channel* channel)
{
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_SYSERR << "epoll_ctl op=" << operation << " fd=" << fd;
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl op=" << operation << " fd=" << fd;
        }
    }
}

} // namespace net

} // namespace tesla
