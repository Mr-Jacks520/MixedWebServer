#include <Connection.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <stdio.h>
#include <errs.h>

Connection::Connection(EventLoop* loop, Socket* sock): _loop(loop), _sock(sock)
{
    _connChannel = new Channel(_loop, _sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this);
    _connChannel->setReadCallback(cb);
    _connChannel->enableReading();
    _connChannel->useET();
    // 初始化缓冲区
    _readBuffer = sdsempty();
    _writeBuffer = sdsempty();
}

Connection::~Connection()
{
    delete _sock;
    delete _connChannel;
    // 释放
    sdsfree(_readBuffer);
    sdsfree(_writeBuffer);
}

void Connection::echo() {
    char buf[BUFFER_SIZE];
    int sockFd = _sock->getFd();
    while (true)
    {
        bzero(&buf, sizeof(buf));
        ssize_t ret = read(sockFd, buf, sizeof(buf));
        if (ret > 0)
        {
            _readBuffer = sdscat(_readBuffer, buf);
        }
        else if (ret < 0 && errno == EINTR) {
            printf("client interupt, conitnue reading...\n");
            continue;
        }else if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            printf("Message from client %d: %s.\n", sockFd, _readBuffer);
            errif(write(_sock->getFd(), _readBuffer, sdslen(_readBuffer)) == -1, "socket write error...");
            sdsclear(_readBuffer);
            break;
        }
        else if (ret == 0)
        {
            printf("EOF, client %d disconnedted...\n", sockFd);
            deleteConnection();
            break;
        }
    }
}


void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> cb) {
    _deleteConnectionCallback = cb;
}

void Connection::deleteConnection() {
    _deleteConnectionCallback(_sock);
}

