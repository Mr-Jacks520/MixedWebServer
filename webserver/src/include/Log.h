#ifndef _LOG_H
#define _LOG_H

#include <BlockQueue.h>
#include <sds.h>
#include <thread>
#include <mutex>
#include <sys/time.h>
#include <sys/stat.h>
#include <string>
#include <cstdarg>

/**
 * @brief 日志等级定义
 *
 */
#define DEBUG 0
#define INFO 1
#define WARN 2
#define ERROR 3

/**
 * @brief 日志类
 *
 */
class Log
{
private:
    FILE *_fp;
    std::unique_ptr<BlockQueue<std::string>> _deque;
    std::unique_ptr<std::thread> _writeThread;
    std::mutex _lock; // lock target: file pointer

    bool _isAsync; // 是否开启异步写

    sds _buffer;  // 日志Buffer
    bool _isOpen; // 是否开启日志

    int _maxLines; // 日志最大写入行数
    int _curLines; // 当前日志写入行数

    int _curDay; // 当前日志记录日期

    const char *_logPath; // 日志文件存储路径
    const char *_suffix;  // 日志文件名后缀

    int _level; // 日志记录等级

    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    Log(/* args */);
    virtual ~Log();

    void _appendLogTitle(int level); // 追加对应日志等级头

    void _asyncWrite(); // 异步写日志
public:
    Log(const Log &) = delete;

    static Log *getInstance();
    void init(int level, const char *path = "./log", const char *suffix = ".log", int maxQueueCapacity = 1024);

    bool IsOpen() { return this->_isOpen; }

    void setLevel(int level);
    int getLevel();

    void write(int level, const char *format, ...);

    void flush();
};

#define LOG_BASE(level, format, ...)                   \
    do                                                 \
    {                                                  \
        Log *log = Log::getInstance();                 \
        if (log->IsOpen() && log->getLevel() <= level) \
        {                                              \
            log->write(level, format, ##__VA_ARGS__);  \
            log->flush();                              \
        }                                              \
    } while (0);

#define LOG_DEBUG(format, ...)                 \
    do                                         \
    {                                          \
        LOG_BASE(DEBUG, format, ##__VA_ARGS__) \
    } while (0);
    
#define LOG_INFO(format, ...)                 \
    do                                        \
    {                                         \
        LOG_BASE(INFO, format, ##__VA_ARGS__) \
    } while (0);

#define LOG_WARN(format, ...)                 \
    do                                        \
    {                                         \
        LOG_BASE(WARN, format, ##__VA_ARGS__) \
    } while (0);

#define LOG_ERROR(format, ...)                 \
    do                                         \
    {                                          \
        LOG_BASE(ERROR, format, ##__VA_ARGS__) \
    } while (0);
    
#define LOG_ERRORIF(cond, format, ...)             \
    do                                             \
    {                                              \
        if (cond)                                  \
        {                                          \
            LOG_BASE(ERROR, format, ##__VA_ARGS__) \
        }                                          \
    } while (0);

#endif // _LOG_H