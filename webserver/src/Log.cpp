#include <Log.h>
#include <cstdio>

Log::Log() {
    _level = 0;
    _fp = nullptr;
    _isAsync = false;
    _buffer = sdsempty();
    _deque = nullptr;
    _writeThread = nullptr;
    _curLines = 0;
    _curDay = 0;
}

Log::~Log() {
    if (_fp) {
        std::unique_lock<std::mutex> locker(_lock);
        flush();
        fclose(_fp);
    }
    if (_writeThread && _writeThread->joinable()) {
        while (!_deque->isEmpty()) {
            _deque->flush();
        }
        _deque->close();
    }
}

/**
 * @brief 单例模式获取日志实例
 * 
 * @return Log* 
 */
Log* Log::getInstance() {
    static Log log; // static variable can only assign once after c++ 11
    return &log;
}

void Log::init(int level, const char* path, const char* suffix, int maxQueueCapacity) {
    _isOpen = true;
    _level = level;
    if (maxQueueCapacity >= 1) {
        _isAsync = true;
        if (!_deque) {
            std::unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>(maxQueueCapacity));
            _deque = std::move(newQueue);

            std::unique_ptr<std::thread> newThread(new std::thread([this]{
                this->getInstance()->_asyncWrite();
            }));
            _writeThread = std::move(newThread);
        }
    }else {
        _isAsync = false;
    }

    _curLines = 0;
    _logPath = path;
    _suffix = suffix;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;

    sds fileName = sdsempty();
    // 填充文件名
    // snprintf(fileName, LOG_PATH_LEN - 1, "%s/%04d_%02d_%02d%s", _logPath, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, _suffix);
    fileName = sdscatprintf(fileName, "%s/%04d_%02d_%02d%s", _logPath, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, _suffix);

    // record current date
    _curDay = t.tm_mday;

    {
        std::unique_lock<std::mutex> locker(_lock);
        if (_fp) {
            flush();
            fclose(_fp);
        }
        _fp = fopen(fileName, "a");
        if (_fp == nullptr) {
            int ret = mkdir(_logPath, 0777);
            _fp = fopen(fileName, "a");
        }
        sdsfree(fileName);
        errif(_fp == nullptr, "logger file create fail in Log.cpp:85");
    }
}

void Log::write(int level, const char* format, ...) {
    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    va_list args;

    // log split by day or block
    if (_curDay != t.tm_mday || (_curLines && (_curLines % MAX_LINES == 0))) {

        std::unique_lock<std::mutex> locker(_lock);

        locker.unlock();

        sds newFileName = sdsempty();
        sds block = sdsempty();
        block = sdscatprintf(block, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        // new log file
        if (_curDay != t.tm_mday) {
            newFileName = sdscatprintf(newFileName, "%s/%s%s", _logPath, block, _suffix);
            _curDay = t.tm_mday;
            _curLines = 0;
        }else {
            newFileName = sdscatprintf(newFileName, "%s/%s-%d%s", _logPath, block, (_curLines / MAX_LINES), _suffix);
        }

        locker.lock();
        flush();
        fclose(_fp);
        _fp = fopen(newFileName, "a");
        sdsfree(newFileName);
        sdsfree(block);
        errif(_fp == nullptr, "new file open fail in Log.cpp:120");
    }

    {
        std::unique_lock<std::mutex> locker(_lock);
        locker.unlock();
        _curLines++;
        _buffer = sdscatprintf(_buffer, "%04d-%02d-%02d %02d:%02d:%02d ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
        // append log title
        _appendLogTitle(_level);

        va_start(args, format);
        // append log info
        _buffer = sdscatvprintf(_buffer, format, args);
        va_end(args);
        // append line break
        sdscat(_buffer, "\n\0");

        if (_isAsync && _deque && !_deque->isFull()) {
            // async write
            _deque->push_back(std::string(_buffer));
        }else {
            // avoid multiple thread to write log file
            locker.lock();
            fputs(_buffer, _fp);
        }
        // swipe space for log buffer
        sdsclear(_buffer);
    }
}

void Log::setLevel(int level) {
    std::lock_guard<std::mutex> locker(_lock);
    _level = level;
}

int Log::getLevel() {
    std::lock_guard<std::mutex> locker(_lock);
    return _level;
}

void Log::flush() {
    if (_isAsync) {
        _deque->flush();
    }
    fflush(_fp);
}

void Log::_appendLogTitle(int level) {
    switch (level)
    {
    case DEBUG:
        _buffer = sdscat(_buffer, "[debug] : ");
        break;
    case INFO:
        _buffer = sdscat(_buffer, "[info] : ");
        break;
    case WARN:
        _buffer = sdscat(_buffer, "[warn] : ");
        break;
    case ERROR:
        _buffer = sdscat(_buffer, "[error] : ");
        break;
    default:
        _buffer = sdscat(_buffer, "[info] : ");
        break;
    }
}

void Log::_asyncWrite() {
    std::string s = "";
    while (_deque->pop(s))
    {
        std::lock_guard<std::mutex> locker(_lock);
        fputs(s.c_str(), _fp);
    }
}
