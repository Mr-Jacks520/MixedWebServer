#ifndef _SQLRAII_H
#define _SQLRAII_H

#include <SqlPool.h>

class SqlRAII
{
private:
    SqlPool *_sqlpool;
    MYSQL *_sql;

public:
    SqlRAII(MYSQL **sql, SqlPool *pool)
    {
        assert(pool);
        *sql = pool->GetConn();
        _sqlpool = pool;
        _sql = *sql;
    }
    ~SqlRAII()
    {
        if (_sql)
        {
            _sqlpool->FreeConn(_sql);
        }
    }
};
#endif // _SQLRAII_H