#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H
#include <Epoll.h>
#include <ThreadPool.h>

class EventLoop
{
private:
    Epoll *_ep;
    bool _exit;
public:
    EventLoop(/* args */);
    ~EventLoop();

    void loop();
    void updateChannel(Channel*);
    void Exit();
};

#endif  // _EVENTLOOP_H