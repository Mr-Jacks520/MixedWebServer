#ifndef _HTTPREQUEST_H
#define _HTTPREQUEST_H

#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <mysql/mysql.h>
#include <algorithm>

#include <SqlRAII.h>
#include <Log.h>

class HttpRequest
{
public:
    enum PARSE_STATE
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };
    
    void Init();

    bool Parse(sds buf);

    const std::string GetMethod();
    const std::string GetPath();
    const std::string GetPost(const char *key);

    bool IsKeepAlive();

    HttpRequest() { Init(); };
    ~HttpRequest() = default;

private:
    bool _ParseRequestLine(const std::string &line);
    void _ParseHeaders(const std::string &line);
    void _ParseBody(const std::string &line);

    void _ParsePath();
    void _ParsePost();
    void _ParseFromUrlencoded();

    int ConvertHex(char c);

    bool UserVerify(const std::string &username, const std::string &pwd, const bool isLogin);

    PARSE_STATE _state;

    std::string _request_method, _request_path, _http_version, _request_body;
    std::unordered_map<std::string, std::string> _request_headers;
    std::unordered_map<std::string, std::string> _post;

    const static std::unordered_set<std::string> DEFAULT_HTMLS;
    const static std::unordered_map<std::string, uint8_t> HTML_TAGS;
};

#endif // _HTTPREQUEST_H