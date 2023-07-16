#include <WebServer.h>
#include <cstring>
#include <unistd.h>
#include <errs.h>
#include <cerrno>
#include <cstdio>
#include <EventLoop.h>
#include <Channel.h>

WebServer::WebServer(uint16_t port, bool openLog, int logLevel, int logSize)
{
    // log initialize
    if (openLog) {
        Log::getInstance()->init(logLevel, "./log", ".logg", logSize);
        LOG_DEBUG("Logger start...");
    }
    _mainReactor = new EventLoop();
    _acceptor = new Acceptor(_mainReactor);
    std::function<void(Socket *)> cb = std::bind(&WebServer::handleConnection, this, std::placeholders::_1);
    _acceptor->setConnectionCallback(cb);

    int size = std::thread::hardware_concurrency();
    _pool = new ThreadPool(size);
    for (int i = 0; i < size; ++i) {
        _subReactors.push_back(new EventLoop());
    }

    // per loop per thread
    for (int i = 0; i < size; ++i) {
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, _subReactors[i]);
        _pool->addTask(sub_loop);
    }
}

WebServer::~WebServer()
{
    delete _mainReactor;
    delete _acceptor;
    delete _pool;
}

void WebServer::start()
{
    // 主Reactor循环
    _mainReactor->loop();
}

/**
 * @brief 处理客户端连接
 *
 */
void WebServer::handleConnection(Socket *sock)
{
    if (sock->getFd() != -1)
    {
        // 随机调度到一个Reactor执行
        int random = sock->getFd() % _subReactors.size();
        Connection *conn = new Connection(_subReactors[random], sock);
        // 设定删除连接回调函数
        std::function<void(Socket *)> cb = std::bind(&WebServer::deleteConnection, this, std::placeholders::_1);
        conn->SetDeleteConnectionCallback(cb);
        // 添加至Map
        _map[sock->getFd()] = conn;
    }
}

/**
 * @brief 断开TCP连接
 *
 * @param sock 客户端套接字
 */
void WebServer::deleteConnection(Socket *sock)
{
    if (sock == nullptr) return;    // solve segmentation fault.--------wait for fix
    int fd = sock->getFd();
    if (fd != -1)
    {
        auto it = _map.find(fd);
        if (it != _map.end())
        {
            Connection *conn = _map[fd];
            _map.erase(fd);
            delete conn;
        }
    }
}
