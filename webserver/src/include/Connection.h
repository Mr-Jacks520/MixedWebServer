#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <EventLoop.h>
#include <Socket.h>
#include <InetAddress.h>
#include <functional>
#include <sds.h>

#define BUFFER_SIZE 1024

class Connection
{
private:
    EventLoop *_loop;
    Socket *_sock;
    Channel *_connChannel;

    std::function<void(Socket*)> _deleteConnectionCallback;
    std::function<void(Connection*)> _onConnectionCallback;

    // 读写缓冲区
    sds _readBuffer;
    sds _writeBuffer;
    
public:
    Connection(EventLoop*, Socket*);
    ~Connection();

    void echo();

    void SetDeleteConnectionCallback(std::function<void(Socket*)>);
    void DeleteConnection();
    void SetOnConnectionCallback(std::function<void(Connection*)> const &cb);
};

#endif  // _CONNECTION_H