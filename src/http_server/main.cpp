#include <asyncio/socket/WSAStartup.h>
#include <asyncio/context.h>
#include <asyncio/buffer/ContainerBufferSender.h>
#include "asyncio/http/http_server.h"

void main() {

    AsyncioGlobal startup;
    startup.start();

    Context context;
    HttpServer server(context, [](auto buffer, auto &socket) {
        std::string response = "HTTP/1.1 200 OK\r\nServer: WebServer\r\nContent-Type: "
                               "text/html\r\nContent-Length: 4\r\nConnection: close\r\n\r\nyeet";

        socket.write(std::make_unique<ContainerBufferSender<std::string>>(std::move(response)),
                     [](auto buffer, bool error) {});
    });

    auto server_good = server.start();

    context.run();
}
