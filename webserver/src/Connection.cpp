#include <Connection.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <stdio.h>
#include <cassert>
#include <errs.h>

Connection::Connection(EventLoop *loop, Socket *sock) : _loop(loop), _sock(sock)
{
    if (_loop != nullptr)
    {
        _connChannel = new Channel(_loop, _sock->GetFd());
        _connChannel->enableReading();
        _connChannel->useET();
    }
    // 初始化缓冲区
    _readBuffer = sdsempty();
    _writeBuffer = sdsempty();
    _state = State::Connected;
}

Connection::~Connection()
{
    if (_loop)
    {
        delete _connChannel;
    }
    delete _sock;
    // 释放
    sdsfree(_readBuffer);
    sdsfree(_writeBuffer);
}

void Connection::echo()
{
    Read();
    Write();
}

Connection::State Connection::GetState()
{
    return _state;
}

Socket *Connection::GetSocket()
{
    return _sock;
}

void Connection::Read()
{
    assert(_state != State::Closed);
    sdsclear(_readBuffer);
    if (_sock->IsNoBlocking())
    {
        ReadNoBlocking();
    }
    else
    {
        ReadBlocking();
    }
}

void Connection::ReadNoBlocking()
{
    char buf[BUFFER_SIZE] = {0};
    int sockFd = _sock->GetFd();
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        ssize_t read_bytes = read(sockFd, buf, sizeof(buf));
        if (read_bytes > 0)
        {
            _readBuffer = sdscat(_readBuffer, buf);
        }
        else if (read_bytes == -1 && errno == EINTR)
        {
            fprintf(stderr, "[non-blocking] ==> continue reading...\n");
            continue;
        }
        else if (read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            fprintf(stderr, "[non-blocking] ==> read over...\n");
            break;
        }
        else if (read_bytes == 0)
        {
            fprintf(stderr, "[non-blocking] ==> read EOF, client fd %d disconnected.\n", sockFd);
            _state = State::Closed;
            break;
        }
        else
        {
            fprintf(stderr, "[non-blocking] ==> other errors occured in client fd %d: %m\n", sockFd);
            _state = State::Closed;
            break;
        }
    }
}

void Connection::ReadBlocking()
{
    int sockFd = _sock->GetFd();
    unsigned int rcv_size = 0;
    socklen_t len = sizeof(rcv_size);
    getsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, &rcv_size, &len);
    char buf[rcv_size] = {0};
    ssize_t read_bytes = read(sockFd, buf, sizeof(buf));
    if (read_bytes > 0)
    {
        _readBuffer = sdscat(_readBuffer, buf);
    }
    else if (read_bytes == 0)
    {
        fprintf(stderr, "[blocking] ==> read EOF, client fd %d disconnected.\n", sockFd);
        _state = State::Closed;
    }
    else if (read_bytes == -1)
    {
        fprintf(stderr, "[blocking] ==> other errors occured in client fd %d: %m\n", sockFd);
        _state = State::Closed;
    }
}

void Connection::Write()
{
    assert(_state != State::Closed);
    if (_sock->IsNoBlocking())
    {
        WriteNoBlocking();
    }
    else
    {
        WriteBlocking();
    }
    sdsclear(_writeBuffer);
}

void Connection::WriteNoBlocking()
{
    int sockFd = _sock->GetFd();
    sds buf = sdsdup(_writeBuffer);
    size_t data_size = sdslen(buf);
    size_t data_left = data_size;
    while (data_left > 0)
    {
        ssize_t write_bytes = write(sockFd, buf + data_size - data_left, data_left);
        if (write_bytes == -1 && errno == EINTR)
        {
            fprintf(stderr, "[non-blocking] ==> continue writing...\n");
            continue;
        }
        else if (write_bytes == -1 && errno == EAGAIN)
        {
            fprintf(stderr, "[non-blocking] ==> write over...\n");
            break;
        }
        else if (write_bytes == -1)
        {
            fprintf(stderr, "[non-blocking] ==> other error occured in client fd %d: %m.\n", sockFd);
            _state = State::Closed;
            break;
        }
        data_left = data_size - write_bytes;
    }
    sdsfree(buf);
}

void Connection::WriteBlocking()
{
    int sockFd = _sock->GetFd();
    ssize_t write_bytes = write(sockFd, _writeBuffer, sdslen(_writeBuffer));
    if (write_bytes == -1)
    {
        fprintf(stderr, "[blocking] ==> other error occured in client fd %d: %m.\n", sockFd);
        _state = State::Closed;
    }
}

void Connection::SetWriteBuffer(const char *str)
{
    if (!sdslen(_writeBuffer))
    {
        sdsclear(_writeBuffer);
    }
    _writeBuffer = sdscat(_writeBuffer, str);
}

sds Connection::GetWriteBuffer()
{
    return _writeBuffer;
}

sds Connection::GetReadBuffer()
{
    return _readBuffer;
}

void Connection::SetDeleteConnectionCallback(std::function<void(Socket *)> const &cb)
{
    _deleteConnectionCallback = cb;
}

void Connection::Close()
{
    _deleteConnectionCallback(_sock);
}

void Connection::SetOnConnectionCallback(std::function<void(Connection *)> const &cb)
{
    _onConnectionCallback = cb;
    _connChannel->setReadCallback([this]()
                                  { _onConnectionCallback(this); });
}
