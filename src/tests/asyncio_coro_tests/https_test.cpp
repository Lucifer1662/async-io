#include <gtest/gtest.h>

#include <asyncio_coro/async_task.h>
#include <asyncio_coro/timer_context.h>
#include <asyncio_coro/socket_context.h>
#include <asyncio_coro/interval.h>
#include <asyncio_coro/linked_socket_context.h>
#include <iostream>
#include <asyncio_coro/ssl/ssl_context.h>
#include <asyncio_coro/https/fetch.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void OpenConnection(int sd, const char *hostname, int port) {
    struct hostent *host;
    struct sockaddr_in addr;
    if ((host = gethostbyname(hostname)) == NULL) {
        perror(hostname);
        abort();
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long *)(host->h_addr);
    if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(sd);
        perror(hostname);
        abort();
    }
}

SSL_CTX *InitCTX(void) {
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();     /* Load cryptos, et.al. */
    SSL_load_error_strings();         /* Bring in and register error messages */
    method = TLSv1_2_client_method(); /* Create new client-method instance */
    ctx = SSL_CTX_new(method);        /* Create new context */
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}
void ShowCerts(SSL *ssl) {
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if (cert != NULL) {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line); /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);      /* free the malloc'ed string */
        X509_free(cert); /* free the malloc'ed certificate copy */
    } else
        printf("Info: No client certificates configured.\n");
}
static AsyncTask<> https_test1(SocketContext &socketContext) {

    SSL_CTX *ctx;
    SSL *ssl;
    char buf[1024];
    int bytes;
    const char *hostname, *portnum;
    SSL_library_init();
    hostname = "www.google.com";
    portnum = "443";
    ctx = InitCTX();
    int server = socket(PF_INET, SOCK_STREAM, 0);

    OpenConnection(server, hostname, atoi(portnum));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server); /* attach the socket descriptor */

    if (SSL_connect(ssl) == -1) /* perform the connection */
        ERR_print_errors_fp(stderr);
    else {
        char acUsername[16] = {0};
        char acPassword[16] = {0};

        const char *cpRequestMessage = "GET / HTTP/1.1\r\nHost: www.google.com\r\nUser-Agent: \r\nAccept : */*\r\n\r\n";
        // "GET / HTTP/2\r\nHost: www.google.com\r\nuser-agent: curl/7.74.0\r\naccept : */*\r\n\r\n";
        ShowCerts(ssl);                                             /* get any certs */
        SSL_write(ssl, cpRequestMessage, strlen(cpRequestMessage)); /* encrypt & send message */
        bytes = SSL_read(ssl, buf, sizeof(buf));                    /* get reply & decrypt */
        printf("Received: \"%s\"\n", buf);
        SSL_free(ssl); /* release connection state */
    }
    close(server);     /* close socket */
    SSL_CTX_free(ctx); /* release context */
}

static AsyncTask<> https_test(SocketContext &socketContext, SSLContext &sslContext) {

    Http::FetchParams params;
    // params.url.host = "www.google.com";
    params.url.host = "www.google.com";
    params.url.protocol = Http::Protocol::HTTPS;
    // params.url.host = "127.0.0.1";
    params.url.path = "/";
    try {
        auto response = co_await Https::fetch(params, socketContext, sslContext);
        std::cout << response.str << std::endl;
        std::cout << response.body << std::endl;

    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}

TEST(Https_Test, Fetch) {
    TimerContext context;
    SocketContext socketContext;
    SSLContext sslContext;

    link_socket_to_timer_context(context, socketContext, std::chrono::milliseconds(100));

    https_test(socketContext, sslContext);
    // https_test1(socketContext);

    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
}
