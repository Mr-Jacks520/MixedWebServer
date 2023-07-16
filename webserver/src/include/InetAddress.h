#ifndef _INETADDRESS_H
#define _INETADDRESS_H
#include <arpa/inet.h>

class InetAddress
{
public:
    sockaddr_in addr;
    socklen_t addrLen;
    InetAddress(/* args */);
    InetAddress(uint16_t port, const char* ip);
    ~InetAddress();
};

#endif  // _INETADDRESS_H