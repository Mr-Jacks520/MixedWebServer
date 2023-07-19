#include <Epoll.h>
#include <unistd.h>
#include <cstring>
#include <errs.h>
#include <cstdio>

#define MAX_EVENTS 1000

Epoll::Epoll(/* args */): _events(nullptr)
{
    _epollFd = epoll_create1(0);
    errif(_epollFd == -1, "epoll create fail.");
    _events = new epoll_event[MAX_EVENTS];
    bzero(_events, sizeof(*_events) * MAX_EVENTS);
}

Epoll::~Epoll()
{
    if (_epollFd != -1) {
        close(_epollFd);
        _epollFd = -1;
    }
    delete[] _events;
}

void Epoll::addFd(int fd, uint32_t op) {
    epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    int ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev);
    errif(ret == -1, "epoll add fail.");
}

std::vector<Channel*> Epoll::getPoll(int timeout) {
    std::vector<Channel*> actives;
    int nfds = epoll_wait(_epollFd, _events, MAX_EVENTS, timeout);
    for (int i = 0; i < nfds; ++i) {
        Channel* ch = (Channel*)_events[i].data.ptr;
        ch->SetRevents(_events[i].events);
        actives.push_back(ch);
    }
    return actives;
}

void Epoll::updateChannel(Channel* channel) {
    epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->isInPoll()) {
        errif(epoll_ctl(_epollFd, EPOLL_CTL_ADD, channel->getFd(), &ev) == -1, "epoll create fail");
        channel->setInPoll();
    }else {
        errif(epoll_ctl(_epollFd, EPOLL_CTL_MOD, channel->getFd(), &ev) == -1, "epoll update fail ");
    }
}

void Epoll::deleteChannel(Channel* ch) {
    errif(epoll_ctl(_epollFd, EPOLL_CTL_DEL, ch->getFd(), NULL) == -1, "epoll del fail...");
    ch->setInPoll();
}
