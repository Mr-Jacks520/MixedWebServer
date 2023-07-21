#include <HttpRequest.h>

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTMLS = {
    "/index", "/login", "/register", "/welcome", "/video", "/image"};

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
    if (sdslen(buf) <= 0)
    {
        return false;
    }
    int read_bytes = 0;
    const size_t BUFFER_SIZE = sdslen(buf);
    while (_state != FINISH)
    {
        const char CRLF[] = "\r\n";
        char *lineEnd = std::search(buf + read_bytes, buf + BUFFER_SIZE, CRLF, CRLF + 2);
        std::string line(buf + read_bytes, lineEnd);
        switch (_state)
        {
        case REQUEST_LINE:
        {
            if (!_ParseRequestLine(line))
            {
                return false;
            }
            _ParsePath();
            break;
        }
        case HEADERS:
        {
            _ParseHeaders(line);
            // 无请求体直接结束解析
            if (BUFFER_SIZE - read_bytes <= 2)
            {
                _state = FINISH;
            }
            break;
        }
        case BODY:
        {
            _ParseBody(line);
            break;
        }
        default:
            break;
        }
        read_bytes += (lineEnd + 2 - (buf + read_bytes));
    }
    LOG_DEBUG("[%s], [%s], [%s]", _request_method.c_str(), _request_path.c_str(), _http_version.c_str());
    return true;
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
    if (_post.count(key) == 1) {
        return _post[key];
    }
    return "";
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
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, pattern))
    {
        _request_method = subMatch[1];
        _request_path = subMatch[2];
        _http_version = subMatch[3];
        _state = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine parse error");
    return false;
}

void HttpRequest::_ParseHeaders(const std::string &line)
{
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, pattern))
    {
        _request_headers[subMatch[1]] = subMatch[2];
    }
    else
    {
        _state = BODY;
    }
}

void HttpRequest::_ParseBody(const std::string &line)
{
    _request_body = line;
    _ParsePost();
    _state = FINISH;
    LOG_DEBUG("Reqeust Body: [%s], length: [%d]", line.c_str(), line.size());
}

void HttpRequest::_ParsePath()
{
    if (_request_path == "/")
    {
        _request_path = "/index.html";
    }
    else
    {
        for (auto &item : DEFAULT_HTMLS)
        {
            if (item == _request_path)
            {
                _request_path += ".html";
                break;
            }
        }
    }
}

void HttpRequest::_ParsePost()
{
    if (_request_method == "POST" && _request_headers["Content-Type"] == "application/x-www-form-urlencoded")
    {
        _ParseFromUrlencoded();
        if (HTML_TAGS.count(_request_path))
        {
            int tag = HTML_TAGS.find(_request_path)->second;
            if (tag == 0 || tag == 1)
            {
                bool isLogin = (tag == 1);
                if (UserVerify(_post["username"], _post["password"], isLogin))
                {
                    _request_path = "/welcome.html";
                }
                else
                {
                    _request_path = "/error.html";
                }
            }
        }
    }
}

int HttpRequest::ConvertHex(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    return c;
}

void HttpRequest::_ParseFromUrlencoded()
{
    if (_request_body.size() <= 0)
        return;

    int i = 0, j = 0;
    uint8_t num = 0;
    std::string key, value;
    int n = _request_body.size();

    while (i < n) {
        switch (_request_body[i])
        {
        case '=':
            key = _request_body.substr(j, i - j);
            j = i + 1;
            break;
        case '&':
            value = _request_body.substr(j, i- j);
            j = i + 1;
            _post[key] = value;
            LOG_DEBUG("[%s]: [%s]", key, value);
            break;
        case '%':
            // handle some control chararter
            num = ConvertHex(_request_body[i + 1]) * 16 + ConvertHex(_request_body[i + 2]);
            _request_body[i + 2] = num % 10;
            _request_body[i + 1] = num / 10;
            i += 2;
            break;
        case '+':
            _request_body[i] = ' ';
            break;
        default:
            break;
        }
        ++i;
    }
    // if encoding end up with empty value e.g: key=<Space>
    if (_post.count(key) == 0 && j < i) {
        value = _request_body.substr(j, i - j);
        _post[key] = value;
        LOG_DEBUG("Empty value for key: [%s]", key);
    }
}

bool HttpRequest::UserVerify(const std::string &username, const std::string &pwd, const bool isLogin)
{
    if (username == "" || pwd == "") return false;
    LOG_DEBUG("username: [%s], password: [%s]", username, pwd);
    MYSQL *sql = nullptr;
    SqlRAII(&sql, SqlPool::GetInstance());

    errif(sql == nullptr, "[MySQL]: Get instance failed.");

    MYSQL_ROW row;
    MYSQL_RES *res = nullptr;
    bool flag = false;
    char SQL[256] = {0};

    if (!isLogin) flag = true;

    // query user
    snprintf(SQL, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", username);

    if (mysql_query(sql, SQL)) {
        mysql_free_result(res);
        return false;
    }

    res = mysql_store_result(sql);
    while((row = mysql_fetch_row(res))) {
        LOG_DEBUG("MYSQL ROW: [%s], [%s]", row[0], row[1]);
        std::string passwd(row[1]);

        if (isLogin) {
            if (passwd == pwd) {
                flag = true;
            }else {
                flag = false;
                LOG_DEBUG("[MYSQL]: pwd error");
            }
        }else {
            flag = false;
            LOG_DEBUG("[MYSQL]: username used");
        }
    }

    mysql_free_result(res);

    // register
    if (!isLogin && flag) {
        LOG_DEBUG("register");
        bzero(SQL, 256);
        snprintf(SQL, 256, "INSERT INTO user(username password) VALUES(%s, %s)", username, pwd);
        LOG_DEBUG("[MYSQL]: %s", SQL);
        if (mysql_query(sql, SQL)) {
            LOG_DEBUG("[MYSQL]: insert error");
            return false;
        }
        flag = true;
    }
    SqlPool::GetInstance()->FreeConn(sql);
    LOG_DEBUG("User verify success");
    return flag;
}