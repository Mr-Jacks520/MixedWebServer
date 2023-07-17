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

    void Bind(InetAddress* targetAddr);
    void Listen();
    int Accept(InetAddress* clientAddr);
    void Connect(InetAddress*);

    void SetNonBlocking();
    bool IsNoBlocking();

    int GetFd();
};

#endif  // _SOCKET_H