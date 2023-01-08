#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

// for windows
struct AsyncioGlobal
{
    WSADATA wsd;
    int nStartup = 0;
    int nErr = 0;

    bool start()
    {
        nErr = WSAStartup(0x0202, &wsd);
        if (nErr)
        {
            WSASetLastError(nErr);
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