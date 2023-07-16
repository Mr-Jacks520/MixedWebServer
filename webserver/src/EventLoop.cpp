#include <EventLoop.h>

EventLoop::EventLoop(/* args */) : _exit(false)
{
    _ep = new Epoll();
}

EventLoop::~EventLoop()
{
    delete _ep;
}

void EventLoop::loop()
{
    while (!_exit)
    {
        std::vector<Channel *> actives = _ep->getPoll();
        for (auto ch : actives)
        {
            ch->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel *ch)
{
    _ep->updateChannel(ch);
}
