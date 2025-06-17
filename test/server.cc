#include "../source/server.hpp"

void OnConnected(const PtrConnection &conn) {
    DEBUG_LOG("NEW CONNECTION:%p", conn.get());
}
void OnClosed(const PtrConnection &conn) {
    DEBUG_LOG("CLOSE CONNECTION:%p", conn.get());
}
void OnMessage(const PtrConnection &conn, Buffer *buf) {
    DEBUG_LOG("%s", buf->ReadPosition());
    buf->RMoveOffset(buf->ReadAbleSize());
    std::string str = "Hello World";
    conn->Send(str.c_str(), str.size());
}
int main()
{
    TcpServer server(8085);
    server.SetThreadCount(2);
    server.EnableInactiveRelease(2);
    server.SetClosedCallback(OnClosed);
    server.SetConnectedCallback(OnConnected);
    server.SetMessageCallback(OnMessage);
    server.Start();
    return 0;
}