#include <Socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errs.h>
#include <sys/socket.h>


Socket::Socket(/* args */)
{
    _sockFd = socket(AF_INET, SOCK_STREAM, 0);
    errif(_sockFd == -1, "socket create error...");
}

Socket::Socket(int fd): _sockFd(fd){
    errif(fd == -1, "socket create error...");
}

Socket::~Socket()
{
    if (_sockFd != -1) {
        close(_sockFd);
    }
}

int Socket::getFd() {
    return this->_sockFd;
}

void Socket::setNonBlocking() {
    fcntl(_sockFd, F_SETFL, fcntl(_sockFd, F_GETFL) | O_NONBLOCK);
}

void Socket::bind(InetAddress* targetAddr) {
    errif(::bind(_sockFd, (sockaddr*)&targetAddr->addr, targetAddr->addrLen) == -1, "socket bind error...");
}

void Socket::listen() {
    errif(::listen(_sockFd, SOMAXCONN) == -1, "socket listen error...");
}

int Socket::accept(InetAddress* clientAddr) {
    int ret = ::accept(_sockFd, (sockaddr*)&clientAddr->addr, &clientAddr->addrLen);
    errif(ret == -1, "socket accept error...");
    return ret;
}

void Socket::connect(InetAddress* addr) {
    errif(::connect(_sockFd, (sockaddr*)&addr->addr, addr->addrLen) == -1, "socker connect error...");
}

