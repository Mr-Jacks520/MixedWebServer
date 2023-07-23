#include <Socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errs.h>
#include <cerrno>
#include <sys/socket.h>

Socket::Socket(/* args */)
{
    _sockFd = socket(AF_INET, SOCK_STREAM, 0);
    errif(_sockFd == -1, "socket create error...");
}

Socket::Socket(int fd) : _sockFd(fd)
{
    errif(fd == -1, "socket create error...");
}

Socket::~Socket()
{
    if (_sockFd != -1)
    {
        close(_sockFd);
    }
}

int Socket::GetFd()
{
    return this->_sockFd;
}

void Socket::SetNonBlocking()
{
    fcntl(_sockFd, F_SETFL, fcntl(_sockFd, F_GETFL) | O_NONBLOCK);
}

bool Socket::IsNoBlocking()
{
    return (fcntl(_sockFd, F_GETFL) & O_NONBLOCK) != 0;
}

void Socket::Bind(InetAddress *targetAddr)
{
    errif(::bind(_sockFd, (sockaddr *)&targetAddr->_addr, targetAddr->_addr_len) == -1, "socket bind error...");
}

void Socket::Listen()
{
    errif(::listen(_sockFd, SOMAXCONN) == -1, "socket listen error...");
}

int Socket::Accept(InetAddress *clientAddr)
{
    int clnt_sockfd = -1;
    struct sockaddr_in tmp_addr
    {
    };
    socklen_t add_len = sizeof(tmp_addr);
    if (IsNoBlocking())
    {
        while (true)
        {
            clnt_sockfd = ::accept(_sockFd, (sockaddr *)&tmp_addr, &add_len);
            if (clnt_sockfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
            {
                continue;
            }
            else if (clnt_sockfd == -1)
            {
                errif(true, "socket accept error.");
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        clnt_sockfd = ::accept(_sockFd, (sockaddr *)&tmp_addr, &add_len);
        errif(clnt_sockfd == -1, "socker accept error.");
    }
    clientAddr->SetAddr(tmp_addr);
    return clnt_sockfd;
}

void Socket::Connect(InetAddress *addr)
{
    struct sockaddr_in tmp_addr = addr->_addr;
    if (IsNoBlocking())
    {
        while (true)
        {
            int ret = ::connect(_sockFd, (sockaddr *)&tmp_addr, sizeof(tmp_addr));
            if (ret == 0)
            {
                break;
            }
            if (ret == -1 && (errno == EINPROGRESS))
            {
                continue;
            }
            if (ret == -1)
            {
                errif(true, "socker connect error.");
            }
        }
    }
    else
    {
        errif(::connect(_sockFd, (sockaddr *)&tmp_addr, sizeof(tmp_addr)) == -1, "socker connect error.");
    }
}
