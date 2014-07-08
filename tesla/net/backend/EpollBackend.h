/*
 *
 * Author:
 *     Thomas Liu
 * Date:
 *     4/17/2014
 * Description:
 *
 *
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_NET_BACKEND_EPOLLBACKEND_H
#define TESLA_NET_BACKEND_EPOLLBACKEND_H

#include <tesla/net/Backend.h>

#include <map>
#include <vector>

struct epoll_event;

namespace tesla
{

namespace net
{

///
/// IO Multiplexing with epoll(4).
///
class EpollBackend
    : public Backend
{
public:
    EpollBackend(EventLoop* loop);
    virtual ~EpollBackend();

    virtual tesla::base::Timestamp run(int timeoutMs, ChannelList* activeChannels);
    virtual void updateChannel(Channel* channel);
    virtual void removeChannel(Channel* channel);

private:
    static const int INIT_EVENTLIST_SIZE = 16;

    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);

    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap;

    int epollfd_;
    EventList events_;
    ChannelMap channels_;
}; // class EpollBackend

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_BACKEND_EPOLLBACKEND_H
