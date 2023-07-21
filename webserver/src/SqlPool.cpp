#include <SqlPool.h>
#include <err.h>

SqlPool::SqlPool(/* args */){}

SqlPool::~SqlPool()
{
    // solve problem: double free
    // this->ClosePool();
}

SqlPool* SqlPool::GetInstance() {
    static SqlPool pool;
    return &pool;
}

void SqlPool::Init(const char* host, unsigned int port, const char* user, const char* pwd, const char* dbName, const int maxconn) {
    assert(maxconn > 0);
    // initialize the MySQL client library before you call any other MySQL function.
    errif(mysql_library_init(0, nullptr, nullptr) < 0, "could not initialize MySQL client library\n");
    // create source for pool
    for (int i = 0; i < maxconn; ++i) {
        MYSQL* sql = nullptr;
        // initializes a MYSQL object suitable for mysql_real_connect()
        sql = mysql_init(sql);
        errif(sql == NULL, "no sufficient memory for MYSQL object\n");
        // establish a connection to a MySQL server running on host
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        errif(sql == NULL, "connection was unsuccessful\n");
        _conns.push(sql);
    }

    this->_MAXCONN = maxconn;
    sem_init(&_sems, 0, maxconn);
    LOG_DEBUG("[MYSQL]: Pool start.");
}

MYSQL* SqlPool::GetConn() {
    MYSQL* sql = nullptr;
    if (_conns.empty()) {
        LOG_ERROR("connn queue is busy.");
        return nullptr;
    }
    sem_wait(&_sems);
    {
        std::lock_guard<std::mutex> lock(_mtx);
        sql = _conns.front();
        _conns.pop();
    }
    return sql;
}

void SqlPool::FreeConn(MYSQL *sql) {
    assert(sql);
    std::lock_guard<std::mutex> lock(_mtx);
    _conns.push(sql);
    sem_post(&_sems);
}

void SqlPool::ClosePool() {
    std::lock_guard<std::mutex> lock(_mtx);
    while (!_conns.empty()) {
        MYSQL *sql = _conns.front();
        _conns.pop();
        mysql_close(sql);
    }
    mysql_library_end();
}

int SqlPool::GetFreeConnSize() {
    return this->_conns.size();
}