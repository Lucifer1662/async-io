#include <asyncio/socket/WSAStartup.h>
#include <asyncio/context.h>
#include <asyncio/buffer/CStringBuffer.h>
#include <asyncio/ContextSocket.h>

#include <list>

void main() {

    AsyncioGlobal startup;
    startup.start();

    Context context;
    std::list<ContextSocket> sockets;

    auto server = ContextListeningSocket(context, [&](ContextSocket &&sock) {
        sockets.emplace_front(sock);
        auto &socket = sockets.front();
        auto it = sockets.begin();
        std::cout << "New Connection: sending hello" << std::endl;
        socket.write(std::make_unique<CStringBufferSender>("hello"), [&](auto buffer, bool error) {
            std::cout << "Wrote hello" << std::endl;
            // sockets.erase(it);
        });
        socket.write(std::make_unique<CStringBufferSender>("hello"), [&](auto buffer, bool error) {
            std::cout << "Wrote hello" << std::endl;
            // sockets.erase(it);
        });
    });

    auto server_good = server.start_listening(1235);

    context.run();
}
