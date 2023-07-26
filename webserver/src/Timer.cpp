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
    _head->next = _head->prev = _tail;
    _tail->prev = _tail->next = _head;
}

void Timer::Add(size_t id, unsigned long timeout, TimeoutCallback &cb)
{
    // create new node
    struct TimerNode *node = (TimerNode *)malloc(sizeof(TimerNode));
    node->id = id;
    node->expires = CLOCK::now() + MS(timeout);
    node->cb = std::move(cb);

    TimerNode *cur = _tail;
    // iteration
    while (cur->prev != _tail && cur > node)
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
    TimerNode* cur = _head;
    while (cur->next != _head)
    {
        if (cur->id == id) {
            cur->expires = CLOCK::now() + MS(timeout);
            return;
        }
    }
    
}

void Timer::Remove(size_t id)
{
    TimerNode *cur = _head;
    while (cur->next != _tail)
    {
        if (cur->id == id) {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            free(cur);
            return;
        }
    }
}