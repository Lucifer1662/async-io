#pragma once
#include <openssl/ssl.h>
#include <openssl/err.h>

class SSLContext {
    SSL_CTX *ctx;
    static bool ssl_libray_inited;

  public:
    SSLContext() {
        if (!ssl_libray_inited) {
            SSL_library_init();
            ssl_libray_inited = true;
        }

        OpenSSL_add_all_algorithms();          /* Load cryptos, et.al. */
        SSL_load_error_strings();              /* Bring in and register error messages */
        auto method = TLSv1_2_client_method(); /* Create new client-method instance */
        ctx = SSL_CTX_new(method);             /* Create new context */
    }

    bool is_good() { return ctx != nullptr; }

    SSLContext(const SSLContext &) = delete;
    SSLContext(SSLContext &&old)
        : ctx(old.ctx) {
        old.ctx = nullptr;
    }

    SSL_CTX *context() { return ctx; }

    ~SSLContext() {
        if (ctx != nullptr) {
            SSL_CTX_free(ctx);
        }
    }
};
