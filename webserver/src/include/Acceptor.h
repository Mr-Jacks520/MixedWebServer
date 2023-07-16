#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H
#include <EventLoop.h>
#include <Socket.h>
#include <InetAddress.h>
#include <functional>

class Acceptor
{
private:
    EventLoop *_loop;
    Socket *_sock;
    Channel* _servChannel;

    std::function<void(Socket*)> _connectionCallback;
public:
    explicit Acceptor(EventLoop* loop);
    ~Acceptor();

    void setConnectionCallback(std::function<void(Socket*)> cb);
    void acceptNewConnection();
};

#endif  // _ACCEPTOR_H