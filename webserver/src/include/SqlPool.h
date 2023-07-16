#ifndef _SQLPOOL_H
#define _SQLPOOL_H

#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <cassert>
#include "Log.h"

class SqlPool
{
private:
    SqlPool(/* args */);
    ~SqlPool();

    int _MAXCONN;

    std::queue<MYSQL*> _conns;
    std::mutex _mtx;
    // initialize for maxconn value
    sem_t _sems;
public:
    static SqlPool* GetInstance();

    void Init(const char* host, unsigned int port, const char* user, const char* pwd, const char* dbName, const int maxconn = 10);

    int GetFreeConnSize();

    void FreeConn(MYSQL*);

    MYSQL* GetConn();

    void ClosePool();
};

#endif  // _SQLPOOL_H