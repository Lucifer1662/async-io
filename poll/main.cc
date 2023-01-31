
#include <chrono>
#include <functional>
#include <vector>
#include <queue>
#include <iostream>
#include "socket.h"
#include "buffer/buffer.h"
#include "buffer/CStringBuffer.h"
#include "buffer/TypeLengthBuffer.h"
#include "context.h"
#include "interval.h"

#include "WSAStartup.h"
#include <stdio.h>

#define DEFAULT_PORT 12345
#define TST_MSG "0123456789abcdefghijklmnopqrstuvwxyz\0"
int main()
{
    TypeLengthBufferReceiver a;
    //sets up winsock, and handles shutting it down by destruction
    AsyncioGlobal asyncIO;
    asyncIO.start();

    Context context;
    std::list<Socket> sockets;
    
    // Call WSAPoll for readability of listener (accepted)
    std::optional<ListeningSocket> server_opt = ListeningSocket::create_listening_socket(DEFAULT_PORT, [&](Socket &&sock)
    {
        sockets.push_back(std::move(sock));
        auto& socket = sockets.back(); 
        printf("Main: Connection established.\n");
        
        socket.register_socket_to_context(context);
        socket.read(std::make_unique<CStringBufferReceiver>(), [](std::unique_ptr<Buffer> buffer_ptr, bool error)
                    { 
                        auto& buffer = *(CStringBufferReceiver*)(buffer_ptr.get());
                        printf("Main: recvd %s bytes\n", buffer.getString().c_str()); 
                    });

    });

    if (server_opt)
    {
        server_opt->register_socket_to_context(context);
    }

    Socket socket;
    socket.connect(DEFAULT_PORT, [](Socket& socket, auto error){
        socket.write(std::make_unique<CStringBufferSender>(TST_MSG), [](auto buffer_ptr, auto error){
            printf("Sent message:%s\n", TST_MSG);
        });
    });

    socket.register_socket_to_context(context);

    make_interval(context, 1000, []() -> bool {
        std::cout << "He" << std::endl;
        return true;
    });

    context.run();
    return 0;
}
