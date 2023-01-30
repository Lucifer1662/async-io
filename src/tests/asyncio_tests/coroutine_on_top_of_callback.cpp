#include <gtest/gtest.h>

#include <coroutine>
#include <asyncio/buffer/buffer.h>
#include <asyncio/buffer/CStringBuffer.h>
#include <asyncio/socket/socket.h>

#include <asyncio/socket/WSAStartup.h>
#include <asyncio/context.h>
#include <asyncio/socket/socket.h>
#include <asyncio/ContextSocket.h>

#include <concepts>
#include <coroutine>
#include <exception>
#include <iostream>

using suspend_never = std::suspend_never;
using suspend_always = std::suspend_always;
template <typename T = void> using coroutine_handle = std::coroutine_handle<T>;

struct noop_coroutine_promise {};
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

struct ReadBytes {};

struct AsyncTask {
    struct promise_type {
        coroutine_handle<> precursor;

        AsyncTask get_return_object() { return {coroutine_handle<promise_type>::from_promise(*this)}; }
        suspend_never initial_suspend() { return {}; }

        auto final_suspend() const noexcept {
            struct awaiter {
                // Return false here to return control to the thread's event loop. Remember that we're
                // running on some async thread at this point.
                bool await_ready() const noexcept { return false; }

                void await_resume() const noexcept {}

                // Returning a coroutine handle here resumes the coroutine it refers to (needed for
                // continuation handling). If we wanted, we could instead enqueue that coroutine handle
                // instead of immediately resuming it by enqueuing it and returning void.
                // coroutine_handle<> await_suspend(coroutine_handle<promise_type> h) {
                void await_suspend(coroutine_handle<promise_type> h) noexcept {
                    auto precursor = h.promise().precursor;
                    if (precursor) {
                        precursor.resume();
                    }
                }
            };
            return awaiter{};
        }
        void unhandled_exception() {}
        AsyncTask return_value() { return {coroutine_handle<promise_type>::from_promise(*this)}; }

        // auto await_transform(ReadBytes reader) {
        //     struct Awaiter {
        //         promise_type &pt;
        //         constexpr bool await_ready() const noexcept { return false; }
        //         Buffer_Ptr await_resume() const noexcept { return std::move(pt.val_in); }
        //         void await_suspend(coroutine_handle<> h) { int w = 0; }
        //     };

        //     return Awaiter{*this};
        // }
    };

    coroutine_handle<promise_type> h_;
    operator coroutine_handle<promise_type>() const { return h_; }
    // A coroutine_handle<promise_type> converts to coroutine_handle<>
    operator coroutine_handle<>() const { return h_; }

    constexpr bool await_ready() const noexcept { return false; }
    void await_resume() noexcept {}
    void await_suspend(coroutine_handle<> coroutine) const noexcept {
        // The coroutine itself is being suspended (async work can beget other async work)
        // Record the argument as the continuation point when this is resumed later. See
        // the final_suspend awaiter on the promise_type above for where this gets used
        h_.promise().precursor = coroutine;
    }
};

template <typename Buffer_T, typename Socket> auto async_read(Socket &socket, std::unique_ptr<Buffer_T> &&buffer) {
    struct Awaitable {
        Socket &socket;
        std::unique_ptr<Buffer_T> buffer;
        constexpr bool await_ready() const noexcept { return false; }
        std::unique_ptr<Buffer_T> await_resume() noexcept { return std::move(buffer); }
        void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
            socket.read(std::move(buffer), [h, this](auto buffer_in, auto error) mutable {
                this->buffer = std::move(buffer_in);
                if (!h.done())
                    h.resume();
            });
        }
    };

    return Awaitable{socket, std::move(buffer)};
}

template <typename Buffer_T, typename Socket> auto async_write(Socket &socket, std::unique_ptr<Buffer_T> &&buffer) {
    struct Awaitable {
        Socket &socket;
        std::unique_ptr<Buffer_T> buffer;

        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
            socket.write(std::move(buffer), [h, this](auto buffer_in, auto error) mutable {
                this->buffer = std::move(buffer_in);
                if (!h.done())
                    h.resume();

                int w = 0;
            });
        }
        std::unique_ptr<Buffer_T> await_resume() noexcept { return std::move(buffer); }

        ~Awaitable() {
            int w = 0;   // foo
        }
    };

    return Awaitable{socket, std::move(buffer)};
}

// template <typename Buffer_T, typename Socket> auto async_write(Socket &socket, std::unique_ptr<Buffer_T>
// &&buffer) {
//     struct Awaitable {
//         struct Data {
//             Socket &socket;
//             std::unique_ptr<Buffer_T> buffer;
//             bool finished = false;
//             coroutine_handle<AsyncTask::promise_type> h = {};

//             Data(Socket &socket)
//                 : socket(socket) {}
//         };

//         std::shared_ptr<Data> data;

//         bool await_ready() const noexcept { return data->finished; }
//         void await_suspend(coroutine_handle<AsyncTask::promise_type> h) { data->h = h; }
//         std::unique_ptr<Buffer_T> await_resume() noexcept { return std::move(data->buffer); }

//         ~Awaitable() {
//             int w = 0;   // foo
//         }
//     };

//     Awaitable awaitable{std::make_shared<Awaitable::Data>(socket)};

//     socket.write(std::move(buffer), [awaitable](auto buffer_in, auto error) mutable {
//         awaitable.data->buffer = std::move(buffer_in);
//         awaitable.data->finished = true;
//         awaitable.data->h.resume();
//         // if (!h.done())
//         // h.resume();
//     });

//     return awaitable;
// }

template <typename Socket> auto async_connect(Socket &socket, const IPAddress &ip) {
    struct Awaitable {
        Socket &socket;
        const IPAddress &ip;
        bool was_successful = false;
        constexpr bool await_ready() const noexcept { return false; }
        bool await_resume() const noexcept { return was_successful; }
        void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
            socket.connect(ip, [h, this](auto &socket, auto error) mutable {
                this->was_successful = !error;
                if (!h.done())
                    h.resume();
            });
        }
    };

    return Awaitable{socket, ip};
}

auto async_accept_new_connection(ContextListeningSocket &socket, int port) {
    struct Awaitable {
        ContextListeningSocket &socket;
        int port;
        std::optional<ContextSocket> new_socket;

        constexpr bool await_ready() const noexcept { return false; }
        ContextSocket await_resume() noexcept {
            auto temp = *new_socket;
            new_socket = {};
            return temp;
        }
        void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
            socket.set_connection_accepted([h, this](ContextSocket &&sock) {
                this->new_socket = std::move(sock);
                if (!h.done())
                    h.resume();
                socket.set_connection_accepted([](auto &&) {});
            });

            // socket.start_listening(port, [h, this](ContextSocket &&sock) {
            //     this->new_socket = std::move(sock);
            //     if (!h.done())
            //         h.resume();
            // });
        }
    };

    return Awaitable{socket, port, {}};
}

AsyncTask counter2(Socket &socket) {

    // for (unsigned i = 0;; ++i) {
    // auto f = co_await ops.read(std::make_unique<CStringBufferReceiver>());
    auto f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
    std::cout << "counter2: " << f->getString() << std::endl;

    f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
    std::cout << "counter2: " << f->getString() << std::endl;
    // }
}

TEST(CoroutineTest, Start) {
    { Socket socket; }

    AsyncioGlobal startup;
    startup.start();
    auto recvBuffer = std::make_unique<CStringBufferReceiver>();

    bool send_called = false;
    bool send1_called = false;
    bool recv_called = false;
    bool recv1_called = false;

    Context context;
    std::list<ContextSocket> sockets;

    // auto server = ContextListeningSocket(context, [&](ContextSocket &&sock) -> AsyncTask {
    //     sockets.emplace_back(sock);
    //     auto &socket = sockets.back();

    //     auto f = co_await async_read(socket.base_socket(), std::make_unique<CStringBufferReceiver>());
    //     std::cout << "counter2: " << f->getString() << std::endl;

    //     f = co_await async_read(socket.base_socket(), std::make_unique<CStringBufferReceiver>());
    //     std::cout << "counter2: " << f->getString() << std::endl;
    // });

    // auto server_good = server.start_listening(1235);
    // ASSERT_TRUE(server_good);

    auto server = ContextListeningSocket(context);
    server.start_listening(1235);

    auto temp1 = [&]() -> AsyncTask {
        for (;;) {
            auto sock = co_await async_accept_new_connection(server, 1235);
            sockets.emplace_back(sock);
            auto &socket = sockets.back();

            auto f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
            std::cout << "counter2: " << f->getString() << std::endl;

            f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
            std::cout << "counter2: " << f->getString() << std::endl;
        }
    };
    temp1();

    auto t = [&]() -> AsyncTask {
        auto socket = ContextSocket(context);
        auto address = IPAddress::from_string("localhost", 1235);
        auto connected_successful = co_await async_connect(socket, *address);

        co_await [&]() -> AsyncTask {
            co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
            int j = 0;
        }();

        co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));

        int j = 0;

        // auto connected_successful = co_await async_connect(socket, *address);
        // auto write1 = async_write(socket, std::make_unique<CStringBufferSender>("hey"));
        // auto write2 = async_write(socket, std::make_unique<CStringBufferSender>("hey"));

        // co_await write1;
        // co_await write2;

        // co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
        // co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
    };
    t();

    // auto socket1 = ContextSocket(context);

    // [&]() -> AsyncTask {
    //     auto connected_successful = co_await async_connect(socket1, *address);
    //     co_await async_write(socket1, std::make_unique<CStringBufferSender>("hey"));
    //     co_await async_write(socket1, std::make_unique<CStringBufferSender>("hey"));
    // }();

    // auto connected_successful = socket.connect(*address, [](auto &socket, auto error) -> AsyncTask {
    //     co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
    //     co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
    // });

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

    // ASSERT_EQ(send_called, true);
    // ASSERT_EQ(send1_called, true);
    // ASSERT_EQ(recv_called, true);
    // ASSERT_EQ(recv1_called, true);
}
