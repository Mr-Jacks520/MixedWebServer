#include <WebServer.h>

int main(int argc, char const *argv[])
{
    WebServer *server = new WebServer(4562, "127.0.0.1", 3307, "root", "root", "webserver", 10, true, 0, 10);
    server->OnConnect([](Connection *conn)
                      {
        if (sdslen(conn->GetReadBuffer()) <= 0)
        {
            return;
        }
        conn->GetHTTPRequest()->Init();
        if (conn->GetHTTPRequest()->Parse(conn->GetReadBuffer())) {
            conn->GetHTTPResponse()->Init(conn->GetHTTPRequest()->GetPath(), conn->GetSrcDir(), conn->GetHTTPRequest()->IsKeepAlive(), 200);
        }else {
            conn->GetHTTPResponse()->Init(conn->GetHTTPRequest()->GetPath(), conn->GetSrcDir(), false, 400);
        }
        sds tmp = conn->GetHTTPResponse()->MakeResponse(conn->GetWriteBuffer());
        conn->SetWriteBuffer(tmp);
        conn->Write(); });
    server->start();
    delete server;
    return 0;
}
