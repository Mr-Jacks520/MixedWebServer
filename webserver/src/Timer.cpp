#include <Timer.h>

Timer::Timer(/* args */)
{
    _init();
}

Timer::~Timer()
{
    TimerNode *cur = _head;
    while (cur->next != nullptr)
    {   
        TimerNode *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    
}

void Timer::_init()
{
    _head = (TimerNode *)malloc(sizeof(TimerNode));
    _tail = (TimerNode *)malloc(sizeof(TimerNode));
    _head->next = _tail;
    _tail->prev = _head;
    _head->prev = _tail->next = nullptr;
}

void Timer::Add(size_t id, unsigned long timeout)
{
    // create new node
    struct TimerNode *node = (TimerNode *)malloc(sizeof(TimerNode));
    node->id = id;
    node->expires = CLOCK::now() + MS(timeout);

    TimerNode *cur = _tail->prev;
    // iteration
    while (cur != _head && *cur > *node)
    {
        cur = cur->prev;
    }

    // insert
    node->prev = cur;
    node->next = cur->next;
    cur->next->prev = node;
    cur->next = node;
}

void Timer::Adjust(size_t id, unsigned long timeout)
{
    TimerNode* cur = _head->next;
    while (cur != _tail)
    {
        if (cur->id == id) {
            cur->expires = CLOCK::now() + MS(timeout);
            return;
        }
    }
    
}

void Timer::Remove(size_t id)
{
    TimerNode *cur = _head->next;
    while (cur != _tail)
    {
        if (cur->id == id) {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            free(cur);
            return;
        }
    }
}

void Timer::Tick() {
    if (_head->next == _tail) {
        return;
    }
    TimerNode* cur = _head->next;
    while(cur != _tail) {
        if (std::chrono::duration_cast<MS>(CLOCK::now() - cur->expires).count() < 0) {
            break;
        }
        _timeoutCallback(cur->id);
        TimerNode *tmp = cur;
        cur = cur->next;
        tmp->prev->next = tmp->next;
        tmp->next->prev = tmp->prev;
        free(tmp);
    }
}

void Timer::SetTimeoutCallback(TimeoutCallback const &cb) {
    _timeoutCallback = cb;
}