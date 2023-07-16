#ifndef _BLOCKQUEUE_H
#define _BLOCKQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <errs.h>

/**
 * @brief 阻塞队列
 * 
 * @tparam T 
 */
template<class T>
class BlockQueue
{
private:
    std::deque<T> _queue;

    bool _isClose;      // 是否关闭阻塞队列
    size_t _capacity;    // 队列容量

    // two condition variable
    std::condition_variable _producerCond; 
    std::condition_variable _consumerCond;

    std::mutex _lock;

public:
    explicit BlockQueue(size_t maxCapacity = 1000);
    ~BlockQueue();

    void push_back(const T &item);
    void push_front(const T &item);
    bool pop(T &item);

    T back();
    T front();

    size_t size();
    size_t capacity();

    bool isEmpty();
    bool isFull();

    void close();

    void flush();

    void clear();
};

template<class T>
BlockQueue<T>::BlockQueue(size_t maxCapacity): _capacity(maxCapacity)
{
    errif(maxCapacity <= 0, "block queue cannot init with empty capacity.");
    _isClose = false;
}

template<class T>
BlockQueue<T>::~BlockQueue()
{
    this->close();
}

template<class T>
void BlockQueue<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(_lock);
    // 队列满则挂起当前线程等待消费者线程唤醒...
    while (_queue.size() >= _capacity)
    {
        _producerCond.wait(locker);
    }
    _queue.push_back(item);
    _consumerCond.notify_one();
}

template<class T>
void BlockQueue<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(_lock);
    // 队列满则挂起当前线程等待消费者线程唤醒...
    while (_queue.size() >= _capacity)
    {
        _producerCond.wait(locker);
    }
    _queue.push_front(item);
    _consumerCond.notify_one();
}

template<class T>
bool BlockQueue<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(_lock);
    // 队列空则挂起当前消费者线程,等待生产者线程唤醒...
    while (_queue.empty())
    {
        _consumerCond.wait(locker);
        if (_isClose) {
            return false;
        }
    }
    item = _queue.front();
    _queue.pop_front();
    _producerCond.notify_one();
    return true;
}

template<class T>
T BlockQueue<T>::front() {
    std::lock_guard<std::mutex> locker(_lock);
    return _queue.front();
}

template<class T>
T BlockQueue<T>::back() {
    std::lock_guard<std::mutex> locker(_lock);
    return _queue.back();
}

template<class T>
size_t BlockQueue<T>::size() {
    std::lock_guard<std::mutex> locker(_lock);
    return _queue.size();
}

template<class T>
size_t BlockQueue<T>::capacity() {
    // std::lock_guard<std::mutex> locker(_lock);
    return _capacity;
}

template<class T>
bool BlockQueue<T>::isEmpty() {
    std::lock_guard<std::mutex> locker(_lock);
    return _queue.empty();
}

template<class T>
bool BlockQueue<T>::isFull() {
    std::lock_guard<std::mutex> locker(_lock);
    return _queue.size() >= _capacity;
}

template<class T>
void BlockQueue<T>::close() {
    {
        std::lock_guard<std::mutex> locker(_lock);
        _queue.clear();
        _isClose = true;
    }
    _consumerCond.notify_all();
    _producerCond.notify_all();
}

template<class T>
void BlockQueue<T>::flush() {
    _consumerCond.notify_one();
}

template<class T>
void BlockQueue<T>::clear() {
    std::lock_guard<std::mutex> locker(_lock);
    _queue.clear();
}

#endif // _BLOCKQUEUE_H