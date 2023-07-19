#include <HttpRequest.h>

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTMLS = {
    "/index", "/login", "/register", "welcome", "/video", "/image"};

const std::unordered_map<std::string, uint8_t> HttpRequest::HTML_TAGS = {
    {"/reigster.html", 0},
    {"/login.html", 1}};

void HttpRequest::Init()
{
    _state = PARSE_STATE::REQUEST_LINE;
    _request_method = _request_path = _request_body = _http_version = "";
    _request_headers.clear();
    _post.clear();
}

bool HttpRequest::Parse(sds buf)
{
    const char CRLF[] = "\r\n";
    if (sdslen(buf) <= 0)
    {
        return false;
    }

    while (_state != FINISH)
    {
        
        switch (_state)
        {
        case REQUEST_LINE:
        {
            break;
        }
        case HEADERS:
        {
            break;
        }
        case BODY:
        {
            break;
        }
        default:
            break;
        }
    }
}

const std::string HttpRequest::GetMethod()
{
    return _request_method;
}

const std::string HttpRequest::GetPath()
{
    return _request_path;
}

const std::string HttpRequest::GetPost(const char *key)
{
}

bool HttpRequest::IsKeepAlive()
{
    if (_request_headers.count("Connection") == 1)
    {
        return _request_headers.find("Connection")->second == "keep-alive" && _http_version == "1.1";
    }
    return false;
}

bool HttpRequest::_ParseRequestLine(const std::string &line)
{
}

void HttpRequest::_ParseHeaders(const std::string &line)
{
}

void HttpRequest::_ParseBody(const std::string &line)
{
}

void HttpRequest::_ParsePath()
{
}

void HttpRequest::_ParsePost()
{
}

void HttpRequest::_ParseFromUrlencoded()
{
}