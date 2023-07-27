#include <Acceptor.h>
#include <stdio.h>
#include <Log.h>


Acceptor::Acceptor(EventLoop* loop): _loop(loop)
{
    // 1. 创建句柄
    _sock = new Socket();
    // 2. 初始化地址信息
    InetAddress* _addr = new InetAddress(8888, "127.0.0.1");
    LOG_DEBUG("server listen in port: %d.", ntohs(_addr->_addr.sin_port));
    // 3. 绑定
    _sock->Bind(_addr);
    // 4. 监听
    _sock->Listen();
    // 5. Channel创建,不用线程池
    _servChannel = new Channel(_loop, _sock->GetFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptNewConnection, this);
    _servChannel->setReadCallback(cb);
    _servChannel->enableReading();
    delete _addr;
}

Acceptor::~Acceptor()
{
    delete _sock;
    delete _servChannel;
}


void Acceptor::setConnectionCallback(std::function<void(Socket*)> cb) {
    _connectionCallback = cb;
}

void Acceptor::acceptNewConnection() {
    InetAddress *client = new InetAddress();
    socklen_t len = client->_addr_len;
    int clientFd = _sock->Accept(client);
    LOG_DEBUG("client connected: %d  %s:%d.", clientFd, inet_ntoa(client->_addr.sin_addr), ntohs(client->_addr.sin_port));
    Socket *clientSock = new Socket(clientFd);
    // 设置非阻塞IO
    clientSock->SetNonBlocking();
    _connectionCallback(clientSock);
    delete client;
}

