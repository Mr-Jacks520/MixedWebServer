#include <InetAddress.h>
#include <string.h>

InetAddress::InetAddress(/* args */)
{
    addrLen = sizeof(addrLen);
    bzero(&addr, addrLen);
}

InetAddress::InetAddress(uint16_t port, const char* ip) {
    addrLen = sizeof(addr);
    bzero(&addr, addrLen);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
}

InetAddress::~InetAddress()
{
}