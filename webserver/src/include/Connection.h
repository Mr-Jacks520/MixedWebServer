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

    // 读写缓冲区
    sds _readBuffer;
    sds _writeBuffer;
    
public:
    Connection(EventLoop*, Socket*);
    ~Connection();

    void echo();

    void setDeleteConnectionCallback(std::function<void(Socket*)>);
    void deleteConnection();
};

#endif  // _CONNECTION_H