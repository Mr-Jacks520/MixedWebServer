# WebServer项目

## 介绍


## 目录结构


## 开发计划

1. echo基本服务器C/S搭建；🫵🏽
2. 线程池搭建; ✋🏼
3. 日志框架搭建；🫱🏻
4. 数据库连接池搭建；🚩
5. HTTP报文解析；
6. 定时器处理不活跃连接；
7. 功能扩展：请求视频、图片、网页等
8. 文件上传存储；
9. 待添加...

## 开发日志

2023-7-19:

1. HTTP解析初探: 一行一行读取解析即可，不必将对应的内容全部对应读取后再进行解析

2023-7-17:

1. 在用户自定义业务逻辑之前将数据先读取到缓冲区, 更符合服务器的特色;

2. None

2023-7-16:

1. Bug find: 第一个客户端连接不会被日志记录;

2. HTTP解析初探;

3. 思考：如何更好地自定义客户端业务处理逻辑

2023-7-15:

1. 数据库连接池搭建;

2. RAII规则构建;

3. 数据库API操作学习;

4. [MYSQL API REFERENCE](https://dev.mysql.com/doc/c-api/8.0/en/c-api-basic-interface-usage.html)

5. 测试通过

2023-5-27:

1. Log system building with sds: async write or sync write, singleton design pattern.

2. Bug continue: Bad file descriptor;

2023-5-25:

1. Bug fix: Segmentation fault: code line 54 in Webserver.cpp;

2. Bug find: Bad file descriptor;

3. Bug find: client cannot recv message from server because receive buffer overflow(maybe) or other cause.

4. 日志系统设计初探

2023-5-20:

1. 基本的单线程服务器搭建；

2. SDS引入缓冲区；

3. Acceptor+Connection创建解耦;

4. 生命周期梳理；

5. 线程池添加;

2023-5-19:

1. Channel添加，将epoll_event的data指针指向某个Channel,使得可监听事件多元化；

2. 事件驱动模式设计：设置回调函数以处理相应的事件，但是目前未将其多线程化；


2023-5-16:

1. 套接字地址信息封装;

2. Epoll多路复用封装;

3. 文件描述符非阻塞添加;

2023-5-14:

1. Socket基本流程搭建

2023-5-12:

1. 项目结构搭建

2. 开发计划制定

3. 个人技术总结
