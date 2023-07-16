#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool
{
private:
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _tasks;
    std::mutex _mtx;
    std::condition_variable _cnv;
    bool _stop;
public:
    ThreadPool(int size = 10);
    ~ThreadPool();

    template<class F, class... Args>
    auto addTask(F &&f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;
};


template<class F, class... Args>
auto ThreadPool::addTask(F &&f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{
    // 返回值类型
    using returnType = typename std::result_of<F(Args...)>::type;
    // 智能指针
    auto task = std::make_shared< std::packaged_task<returnType()> >(
        // 完美转发
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<returnType> res = task->get_future();
    {
        std::lock_guard<std::mutex> locker(_mtx);
        _tasks.emplace([task]{
            (*task)();
        });
    }
    _cnv.notify_one();
    return res;
}


#endif  // _THREADPOOL_H