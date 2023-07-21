#include <WebServer.h>

int main(int argc, char const *argv[])
{
    WebServer *server = new WebServer(4562, "127.0.0.1", 3307, "root", "root", "webserver", 10, true, 1, 1024);
    server->OnConnect([](Connection *conn)
                      {
        fprintf(stderr, "Message from client %d: %s.\n", conn->GetSocket()->GetFd(), conn->GetReadBuffer());
        conn->SetWriteBuffer(conn->GetReadBuffer());
        conn->Write(); });
    server->start();
    delete server;
    return 0;
}
