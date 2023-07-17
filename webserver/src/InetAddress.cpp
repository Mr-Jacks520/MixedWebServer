#include <InetAddress.h>
#include <string.h>

InetAddress::InetAddress(/* args */)
{
    _addr_len = sizeof(_addr_len);
    bzero(&_addr, _addr_len);
}

InetAddress::InetAddress(uint16_t port, const char* ip) {
    _addr_len = sizeof(_addr);
    bzero(&_addr, _addr_len);
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(ip);
    _addr.sin_port = htons(port);
}

InetAddress::~InetAddress()
{
}

void InetAddress::SetAddr(struct sockaddr_in addr) {
    _addr = addr;
}

const char* InetAddress::GetIP() {
    return inet_ntoa(_addr.sin_addr);
}

uint16_t InetAddress::GetPort() {
    return ntohs(_addr.sin_port);
}