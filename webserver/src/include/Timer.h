#ifndef _TIMER_H
#define _TIMER_H

#include <functional>
#include <chrono>

#include "errs.h"
#include "Log.h"

#define CLOCK std::chrono::high_resolution_clock
#define MS std::chrono::milliseconds
#define TimeStamp std::chrono::time_point<CLOCK>
#define TimeoutCallback std::function<void()>

typedef struct TimerNode
{
    size_t id;
    TimeStamp expires;
    TimeoutCallback cb;

    TimerNode *prev;
    TimerNode *next;

    bool operator>(const TimerNode &t)
    {
        return expires > t.expires;
    }
};

class Timer
{
public:
    Timer(/* args */);
    ~Timer();

    void Add(size_t id, unsigned long timeout, TimeoutCallback &cb);    

    void Adjust(size_t id, unsigned long timeout);

    void Remove(size_t id);
private:
    void _init();

    TimerNode *_head;
    TimerNode *_tail;
};

#endif // _TIMER_H