#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <vector>
#include <map>
#include <signal.h>

#include "Socket.h"
#include "Acceptor.h"
#include "Connection.h"
#include "Log.h"
#include "ThreadPool.h"
#include "Timer.h"

class EventLoop;
class Channel;
class WebServer
{
private:
    EventLoop *_mainReactor;
    std::vector<EventLoop*> _subReactors;
    ThreadPool *_pool;

    Acceptor *_acceptor;
    std::map<int, Connection*> _map;

    std::string _srcDir;

    Timer *_tim;

    std::function<void(Connection*)> _on_connect_callback;
    
    void handleConnection(Socket*);

    void deleteConnection(Socket*);

    void _addSig(int sig, void (handler)(int), bool restart);

    static void _sig_handler(int sig);

    // void handleReadEvent(int sockFd);

public:
    explicit WebServer(uint16_t port, const char *dbHost, const unsigned int dbPort, const char *dbUser, const char *dbPwd, const char *dbName, int connPoolNums, bool openLog = true, int logLevel = 1, int logSize = 0);
    ~WebServer();

    void start();

    void OnConnect(std::function<void(Connection*)> const &cb);
};
#endif  // _WEBSERVER_H