#ifndef _HTTPRESPONSE_H
#define _HTTPRESPONSE_H

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <unordered_map>

#include "Log.h"

class HttpResponse
{
public:
    HttpResponse(/* args */);
    ~HttpResponse();

    void Init(std::string path, std::string srcDir, bool isKeepAlive, int code);

    void MakeResponse(sds buf);

    void UnmapFile();

private:
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<uint16_t, std::string> CODE_STATUS;
    static const std::unordered_map<uint16_t, std::string> CODE_PATH;

    char *_mmFile;
    struct stat _mmFileStat;

    std::string _path;
    std::string _srcDir;

    uint16_t _code;
    bool _isKeepAlive;

    void _AddStatusLine(sds buf);
    void _AddHeader(sds buf);
    void _AddContent(sds buf);

    void _ErrorPage();

    void _ErrorContent(sds buf, const char *msg);

    std::string _GetFileType();
};

#endif // _HTTPRESPONSE_H