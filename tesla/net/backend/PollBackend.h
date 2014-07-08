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

#ifndef TESLA_NET_BACKEND_POLLBACKEND_H
#define TESLA_NET_BACKEND_POLLBACKEND_H

#include <tesla/net/Backend.h>

#include <map>
#include <vector>

struct pollfd;

namespace tesla
{

namespace net
{

///
/// IO Multiplexing with poll(2).
///
class PollBackend
    : public Backend
{
public:

    PollBackend(EventLoop* loop);
    virtual ~PollBackend();

    virtual tesla::base::Timestamp run(int timeoutMs, ChannelList* activeChannels);
    virtual void updateChannel(Channel* channel);
    virtual void removeChannel(Channel* channel);

private:
    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;

    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;
    PollFdList pollfds_;
    ChannelMap channels_;
};

} // namespace net

} // namespace tesla

#endif  // TESLA_NET_BACKEND_POLLBACKEND_H
