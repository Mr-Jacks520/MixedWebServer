#include <ThreadPool.h>

ThreadPool::ThreadPool(int size) : _stop(false)
{
    for (int i = 0; i < size; ++i)
    {
        _threads.emplace_back(std::thread([this]
                                          {
                        while (true)
                        {
                            std::function<void()> task;
                            {
                                std::unique_lock<std::mutex> locker(_mtx);
                                _cnv.wait(locker, [this]{
                                    return _stop || !_tasks.empty();
                                });
                                if (_stop && _tasks.empty()) return;
                                task = _tasks.front();
                                _tasks.pop();
                            }
                            task();
                        } }));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> locker(_mtx);
        _stop = true;
    }
    _cnv.notify_all();
    for (std::thread &th : _threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}
