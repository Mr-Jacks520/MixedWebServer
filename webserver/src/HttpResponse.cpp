#include <HttpResponse.h>

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

const std::unordered_map<uint16_t, std::string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<uint16_t, std::string> HttpResponse::CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse(/* args */)
{
    _path = _srcDir = "";
    _isKeepAlive = false;
    _code = -1;
    _mmFile = nullptr;
    _mmFileStat = {0};
}

HttpResponse::~HttpResponse()
{
    UnmapFile();
}

void HttpResponse::Init(std::string path, std::string srcDir, bool isKeepAlive, int code = -1)
{
    if (_mmFile)
        UnmapFile();
    _path = path;
    _srcDir = srcDir;
    _isKeepAlive = isKeepAlive;
    _code = code;
    _mmFile = nullptr;
    _mmFileStat = {0};
}

void HttpResponse::MakeResponse(sds buf)
{
    if (stat((_srcDir + _path).data(), &_mmFileStat) < 0 || S_ISDIR(_mmFileStat.st_mode))
    {
        _code = 404;
    }
    else if (!(_mmFileStat.st_mode & S_IROTH))
    {
        _code = 403;
    }
    _ErrorPage();
    _AddStatusLine(buf);
    _AddHeader(buf);
    _AddContent(buf);
}

void HttpResponse::UnmapFile()
{
    if (_mmFile)
    {
        munmap(_mmFile, _mmFileStat.st_size);
        _mmFile = nullptr;
    }
}

void HttpResponse::_AddStatusLine(sds buf)
{
    std::string status;
    if(CODE_STATUS.count(_code) == 1) {
        status = CODE_STATUS.find(_code)->second;
    }else {
        _code = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buf = sdscatprintf(buf, "HTTP/1.1 %d %s\r\n", _code, status.c_str());
}

void HttpResponse::_AddHeader(sds buf)
{
    buf = sdscat(buf, "Connection: ");
    if (_isKeepAlive) {
        buf = sdscat(buf, "keep-alive\r\nkeep-alive: max=6, timeout=120\r\n");
    }else {
        buf = sdscat(buf, "close\r\n");
    }
    buf = sdscatprintf(buf, "Content-Type: %s\r\n", _GetFileType().c_str());
}

void HttpResponse::_AddContent(sds buf)
{
    int fd = open((_srcDir + _path).data(), O_RDONLY);
    if (fd < 0) {
        _ErrorContent(buf, "file not found");
        return;
    }

    // mmap
    LOG_DEBUG("file path %s", (_srcDir + _path).data());
    int* mmRet = (int*)mmap(0, _mmFileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (*mmRet == -1) {
        _ErrorContent(buf, "file not found");
        return;
    }
    _mmFile = (char*)mmRet;
    close(fd);
    buf = sdscatprintf(buf, "Content-length: %ld\r\n\r\n", _mmFileStat.st_size);

    // Add body
    buf = sdscat(buf, _mmFile);
}

void HttpResponse::_ErrorContent(sds buf, const char *msg)
{
    std::string body;
    std::string status;
    if (CODE_STATUS.count(_code) == 1) {
        status = CODE_STATUS.find(_code)->second;
    }else {
        status = "Bad Request";
    }
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += std::to_string(_code) + " : " + status  + "\n";
    body.append("<p>" + std::string(msg) + "</p>");
    body += "<hr><em>TinyWebServer</em></body></html>";

    buf = sdscatprintf(buf, "Content-length: %d\r\n\r\n", body.size());
    buf = sdscat(buf, body.c_str());
}

void HttpResponse::_ErrorPage()
{
    if(CODE_PATH.count(_code) == 1) {
        _path = CODE_PATH.find(_code)->second;
        stat((_srcDir + _path).data(), &_mmFileStat);
    }
}

std::string HttpResponse::_GetFileType()
{
    size_t suffix_index = _path.find_last_of(".");
    if (suffix_index == std::string::npos) {
        return "text/plain";
    }
    std::string suffix_type = _path.substr(suffix_index);
    if (SUFFIX_TYPE.count(suffix_type) == 1) {
        return SUFFIX_TYPE.find(suffix_type)->second;
    }
    return "text/plain";
}