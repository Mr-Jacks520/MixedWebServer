#include <WebServer.h>
#include <cstring>
#include <unistd.h>
#include <errs.h>
#include <cerrno>
#include <cstdio>
#include <EventLoop.h>
#include <Channel.h>

#define PROJECT_NAME "/webserver"
#define RESOURCES_DIR "/resources"

#define TIME_CYCLE 1000

WebServer::WebServer(uint16_t port, const char *dbHost, const unsigned int dbPort, const char *dbUser, const char *dbPwd, const char *dbName, int connPoolNums, bool openLog, int logLevel, int logSize)
{
    // log initialize
    if (openLog)
    {
        Log::getInstance()->init(logLevel, "./log", ".logg", logSize);
        LOG_DEBUG("Logger start...");
    }
    _mainReactor = new EventLoop();
    _acceptor = new Acceptor(_mainReactor);
    std::function<void(Socket *)> cb = std::bind(&WebServer::handleConnection, this, std::placeholders::_1);
    _acceptor->setConnectionCallback(cb);

    int size = std::thread::hardware_concurrency();
    _pool = new ThreadPool(size);
    for (int i = 0; i < size; ++i)
    {
        _subReactors.push_back(new EventLoop());
    }

    // per loop per thread
    for (int i = 0; i < size; ++i)
    {
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, _subReactors[i]);
        _pool->addTask(sub_loop);
    }

    // get http resourse dir
    std::unique_ptr<char> tmp(nullptr);
    tmp.reset(getcwd(tmp.get(), 256));
    char *index = std::search(tmp.get(), tmp.get() + strlen(tmp.get()), PROJECT_NAME, PROJECT_NAME + 10);
    _srcDir.assign(tmp.get(), index);
    _srcDir.append(PROJECT_NAME + std::string(RESOURCES_DIR));

    LOG_DEBUG("html resources path: %s", _srcDir.c_str());

    // sql pool initialize
    SqlPool::GetInstance()->Init(dbHost, dbPort, dbUser, dbPwd, dbName, connPoolNums);
}

WebServer::~WebServer()
{
    delete _tim;
    delete _mainReactor;
    delete _acceptor;
    if (!_subReactors.empty())
    {
        for (auto item : _subReactors)
        {
            delete item;
        }
    }
    delete _pool;
}

void WebServer::start()
{
    // timer setting
    _tim = new Timer();

    // add signal
    _addSig(SIGALRM, _sig_handler, false);
    _addSig(SIGTERM, _sig_handler, false);

    // trigger SIGALRM cyclically
    alarm(TIME_CYCLE);

    // 主Reactor循环
    _mainReactor->loop();
}

/**
 * @brief 处理客户端连接
 *
 */
void WebServer::handleConnection(Socket *sock)
{
    if (sock->GetFd() != -1)
    {
        // 随机调度到一个Reactor执行
        int random = sock->GetFd() % _subReactors.size();
        Connection *conn = new Connection(_subReactors[random], sock);
        // 设定删除连接回调函数
        std::function<void(Socket *)> cb = std::bind(&WebServer::deleteConnection, this, std::placeholders::_1);
        conn->SetDeleteConnectionCallback(cb);
        // 设定连接建立后用户自定义回调函数
        conn->SetOnConnectionCallback(_on_connect_callback);
        // 初始化HTTP资源location
        conn->SetSrcDir(_srcDir.c_str());
        // 添加至Map
        _map[sock->GetFd()] = conn;
    }
}

/**
 * @brief 断开TCP连接
 *
 * @param sock 客户端套接字
 */
void WebServer::deleteConnection(Socket *sock)
{
    if (sock == nullptr)
        return; // solve segmentation fault.--------wait for fix
    int fd = sock->GetFd();
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

void WebServer::OnConnect(std::function<void(Connection *)> const &cb)
{
    _on_connect_callback = cb;
}

void WebServer::_addSig(int sig, void(handler)(int), bool restart)
{
    struct sigaction act = {0};
    act.sa_handler = handler;
    if (restart)
    {
        act.sa_flags |= SA_RESTART;
    }
    sigfillset(&act.sa_mask);
    errif(sigaction(sig, &act, nullptr) != -1, "sigaction failed");
}

void WebServer::_sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;

    // send

    errno = save_errno;
}