#ifndef _EPOLL_H
#define _EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <Channel.h>

class Epoll
{
private:
    int _epollFd;
    epoll_event *_events;
public:
    Epoll(/* args */);
    ~Epoll();

    void addFd(int fd, uint32_t op);
    std::vector<Channel*> getPoll(int timeout = -1);

    void updateChannel(Channel*);
    void deleteChannel(Channel*);
};

#endif // _EPOLL_H