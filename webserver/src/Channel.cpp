#include <Channel.h>
#include <EventLoop.h>
#include <unistd.h>

Channel::Channel(EventLoop *lp, int fd) : _loop(lp), _fd(fd), _events(0), _revents(0), _inPoll(false)
{
}

Channel::~Channel()
{
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

void Channel::enableReading()
{
    _events |= EPOLLIN | EPOLLPRI;
    // epoll更新
    _loop->updateChannel(this);
}

void Channel::EnableWriting()
{
    _events |= EPOLLOUT;
    _loop->updateChannel(this);
}

void Channel::useET()
{
    _events |= EPOLLET;
    _loop->updateChannel(this);
}

void Channel::SetRevents(uint32_t ev)
{
    _revents = ev;
}

uint32_t Channel::GetRevents()
{
    return _revents;
}

void Channel::setInPoll()
{
    _inPoll = true;
}

bool Channel::isInPoll()
{
    return _inPoll;
}

int Channel::getFd()
{
    return _fd;
}

uint32_t Channel::getEvents()
{
    return _events;
}

void Channel::setReadCallback(std::function<void()> const &callback)
{
    _readCallback = callback;
}

void Channel::setWriteCallback(std::function<void()> const &callback)
{
    _writeCallback = callback;
}

void Channel::handleEvent()
{
    if (_revents & (EPOLLIN | EPOLLPRI))
    {
        _readCallback();
    }
    if (_revents & (EPOLLOUT))
    {
        _writeCallback();
    }
}
