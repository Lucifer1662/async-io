
#include <chrono>
#include <functional>
#include <vector>
#include <queue>
#include "socket.h"
#include "buffer.h"
#include "context.h"

#ifdef _IA64_
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <stdio.h>

#define ERR(e) \
    printf("%s:%s failed: %d [%s@%ld]\n", __FUNCTION__, e, WSAGetLastError(), __FILE__, __LINE__)



#define DEFAULT_WAIT 30000

#define WS_VER 0x0202

#define DEFAULT_PORT 12345

#define TST_MSG "0123456789abcdefghijklmnopqrstuvwxyz\0"

// for windows
struct AsyncioGlobal
{
    WSADATA wsd;
    int nStartup = 0;
    int nErr = 0;

    bool start()
    {
        nErr = WSAStartup(WS_VER, &wsd);
        if (nErr)
        {
            WSASetLastError(nErr);
            ERR("WSAStartup");
            return false;
        }
        else
        {
            nStartup++;
        }

        return true;
    }

    ~AsyncioGlobal()
    {
        if (nStartup)
            WSACleanup();
    }
};


int main()
{
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


    context.run();
    return 0;
}
