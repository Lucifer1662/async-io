#include <asyncio/socket/WSAStartup.h>
#include <asyncio/buffer/CStringBuffer.h>
#include <asyncio/context.h>
#include <asyncio/socket/socket.h>
#include <asyncio/ContextSocket.h>
#include <asyncio/http/http_request_buffer.h>
#include <asyncio/http/fetch.h>

void main() {

    AsyncioGlobal startup;
    startup.start();

    Context context;

    Fetcher fetcher(context);

    FetchParams params;
    params.url.host = "www.google.com";

    fetcher.fetch(std::move(params), [](auto buffer, bool error) {
        int w = 0;
        int j = 0;
    });

    context.run();
}
