#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <EventLoop.h>
#include <Socket.h>
#include <InetAddress.h>
#include <functional>
#include <sds.h>

#include "HttpRequest.h"
#include "HttpResponse.h"

#define BUFFER_SIZE 1024

class Connection
{
public:
    enum State
    {
        Invalid = 1,
        HandShaking,
        Connected,
        Closed,
        Failed
    };

    Connection(EventLoop *, Socket *);
    ~Connection();

    void echo();

    void SetDeleteConnectionCallback(std::function<void(Socket *)> const &cb);
    void Close();
    void SetOnConnectionCallback(std::function<void(Connection *)> const &cb);

    State GetState();

    void SetWriteBuffer(sds buf);
    sds GetWriteBuffer();
    sds GetReadBuffer();
    Socket *GetSocket();

    void Read();
    void Write();

    // 非阻塞读写实现: server use
    void ReadNoBlocking();
    void WriteNoBlocking();

    // 阻塞读写实现: client use
    void ReadBlocking();
    void WriteBlocking();

    void SetSrcDir(const char *dir);
    std::string GetSrcDir();

    HttpRequest* GetHTTPRequest();
    HttpResponse* GetHTTPResponse();

private:
    EventLoop *_loop;
    Socket *_sock;
    Channel *_connChannel;

    std::function<void(Socket *)> _deleteConnectionCallback;
    std::function<void(Connection *)> _onConnectionCallback;

    // 读写缓冲区
    sds _readBuffer;
    sds _writeBuffer;

    State _state{State::Invalid};

    // http response source directory
    std::string _srcDir;

    // http service support
    HttpRequest *_request;
    HttpResponse *_response;
};

#endif // _CONNECTION_H