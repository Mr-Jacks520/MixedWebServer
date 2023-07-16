#ifndef _SOCKET_H
#define _SOCKET_H
#include <InetAddress.h>


class Socket
{
private:
    int _sockFd;
public:
    Socket(/* args */);
    Socket(int fd);
    ~Socket();

    void bind(InetAddress* targetAddr);
    void listen();
    int accept(InetAddress* clientAddr);
    void connect(InetAddress*);

    void setNonBlocking();

    int getFd();
};

#endif  // _SOCKET_H