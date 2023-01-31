// #include <gtest/gtest.h>

// #include <experimental/coroutine>
// #include <asyncio/buffer/buffer.h>
// #include <asyncio/buffer/CStringBuffer.h>
// #include <asyncio/socket/socket.h>

// #include <asyncio/socket/WSAStartup.h>
// #include <asyncio/context.h>
// #include <asyncio/socket/socket.h>
// #include <asyncio/ContextSocket.h>

// #include <concepts>
// #include <coroutine>
// #include <exception>
// #include <iostream>

// using suspend_never = std::experimental::suspend_never;
// using suspend_always = std::experimental::suspend_always;
// template <typename T = void> using coroutine_handle = std::experimental::coroutine_handle<T>;

// struct ReadBytes {};

// struct AsyncTask {
//     struct promise_type {
//         AsyncTask get_return_object() {
//             return {
//                 std::make_shared<coroutine_handle<promise_type>>(coroutine_handle<promise_type>::from_promise(*this))};
//         }
//         suspend_never initial_suspend() { return {}; }
//         suspend_never final_suspend() noexcept { return {}; }
//         void unhandled_exception() {}
//         AsyncTask return_value() {
//             return {
//                 std::make_shared<coroutine_handle<promise_type>>(coroutine_handle<promise_type>::from_promise(*this))};
//         }
//     };

//     std::shared_ptr<coroutine_handle<promise_type>> h_;
//     operator coroutine_handle<promise_type>() const { return *h_; }
//     // A coroutine_handle<promise_type> converts to coroutine_handle<>
//     operator coroutine_handle<>() const { return *h_; }
// };

// template <typename Buffer_T> auto async_read(Context &context, SOCKET fd, std::unique_ptr<Buffer_T> &&buffer) {
//     struct Awaitable {
//         Context &context;
//         SOCKET fd;
//         std::unique_ptr<Buffer_T> buffer;
//         constexpr bool await_ready() const noexcept { return false; }
//         std::unique_ptr<Buffer_T> await_resume() noexcept { return std::move(buffer); }
//         void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
//             context.add_socket(this->fd, [this, h](Flag flag) {
//                 if (flag.isRead()) {
//                     this->context.remove_socket(this->fd);
//                     if (!h.done())
//                         h.resume();
//                 }
//             });
//         }
//     };

//     return Awaitable{context, fd, std::move(buffer)};
// }

// template <typename Buffer_T, typename Socket> auto async_write(Socket &socket, std::unique_ptr<Buffer_T> &&buffer) {
//     struct Awaitable {
//         Socket &socket;
//         std::unique_ptr<Buffer_T> buffer;
//         constexpr bool await_ready() const noexcept { return false; }
//         std::unique_ptr<Buffer_T> await_resume() noexcept { return std::move(buffer); }
//         void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
//             socket.write(std::move(buffer), [h, this](auto buffer_in, auto error) mutable {
//                 this->buffer = std::move(buffer_in);
//                 if (!h.done())
//                     h.resume();
//             });
//         }

//         ~Awaitable() {
//             int w = 0;   // foo
//         }
//     };

//     return Awaitable{socket, std::move(buffer)};
// }

// template <typename Socket> auto async_connect(Socket &socket, const IPAddress &ip) {
//     struct Awaitable {
//         Socket &socket;
//         const IPAddress &ip;
//         bool was_successful = false;
//         constexpr bool await_ready() const noexcept { return false; }
//         bool await_resume() const noexcept { return was_successful; }
//         void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
//             socket.connect(ip, [h, this](auto &socket, auto error) mutable {
//                 this->was_successful = !error;
//                 if (!h.done())
//                     h.resume();
//             });
//         }
//     };

//     return Awaitable{socket, ip};
// }

// auto async_accept_new_connection(ContextListeningSocket &socket, int port) {
//     struct Awaitable {
//         ContextListeningSocket &socket;
//         int port;
//         std::optional<ContextSocket> new_socket;

//         constexpr bool await_ready() const noexcept { return false; }
//         ContextSocket await_resume() noexcept {
//             auto temp = *new_socket;
//             new_socket = {};
//             return temp;
//         }
//         void await_suspend(coroutine_handle<AsyncTask::promise_type> h) {
//             socket.set_connection_accepted([h, this](ContextSocket &&sock) {
//                 this->new_socket = std::move(sock);
//                 if (!h.done())
//                     h.resume();
//                 socket.set_connection_accepted([](auto &&) {});
//             });

//             // socket.start_listening(port, [h, this](ContextSocket &&sock) {
//             //     this->new_socket = std::move(sock);
//             //     if (!h.done())
//             //         h.resume();
//             // });
//         }
//     };

//     return Awaitable{socket, port, {}};
// }

// AsyncTask counter2(Socket &socket) {

//     // for (unsigned i = 0;; ++i) {
//     // auto f = co_await ops.read(std::make_unique<CStringBufferReceiver>());
//     auto f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
//     std::cout << "counter2: " << f->getString() << std::endl;

//     f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
//     std::cout << "counter2: " << f->getString() << std::endl;
//     // }
// }

// TEST(CoroutineTest, Start) {
//     { Socket socket; }

//     AsyncioGlobal startup;
//     startup.start();
//     auto recvBuffer = std::make_unique<CStringBufferReceiver>();

//     bool send_called = false;
//     bool send1_called = false;
//     bool recv_called = false;
//     bool recv1_called = false;

//     Context context;
//     std::list<ContextSocket> sockets;

//     // auto server = ContextListeningSocket(context, [&](ContextSocket &&sock) -> AsyncTask {
//     //     sockets.emplace_back(sock);
//     //     auto &socket = sockets.back();

//     //     auto f = co_await async_read(socket.base_socket(), std::make_unique<CStringBufferReceiver>());
//     //     std::cout << "counter2: " << f->getString() << std::endl;

//     //     f = co_await async_read(socket.base_socket(), std::make_unique<CStringBufferReceiver>());
//     //     std::cout << "counter2: " << f->getString() << std::endl;
//     // });

//     // auto server_good = server.start_listening(1235);
//     // ASSERT_TRUE(server_good);

//     auto server = ContextListeningSocket(context);
//     server.start_listening(1235);

//     auto temp1 = [&]() -> AsyncTask {
//         for (;;) {
//             auto sock = co_await async_accept_new_connection(server, 1235);
//             sockets.emplace_back(sock);
//             auto &socket = sockets.back();

//             auto f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
//             std::cout << "counter2: " << f->getString() << std::endl;

//             f = co_await async_read(socket, std::make_unique<CStringBufferReceiver>());
//             std::cout << "counter2: " << f->getString() << std::endl;
//         }
//     };
//     temp1();

//     auto socket = ContextSocket(context);
//     auto address = IPAddress::from_string("localhost", 1235);

//     [&]() -> AsyncTask {
//         auto connected_successful = co_await async_connect(socket, *address);
//         co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
//         co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
//     }();

//     auto socket1 = ContextSocket(context);

//     [&]() -> AsyncTask {
//         auto connected_successful = co_await async_connect(socket1, *address);
//         co_await async_write(socket1, std::make_unique<CStringBufferSender>("hey"));
//         co_await async_write(socket1, std::make_unique<CStringBufferSender>("hey"));
//     }();

//     // auto connected_successful = socket.connect(*address, [](auto &socket, auto error) -> AsyncTask {
//     //     co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
//     //     co_await async_write(socket, std::make_unique<CStringBufferSender>("hey"));
//     // });

//     context.step();
//     context.step();
//     context.step();
//     context.step();
//     context.step();
//     context.step();
//     context.step();
//     context.step();
//     context.step();
//     context.step();

//     // ASSERT_EQ(send_called, true);
//     // ASSERT_EQ(send1_called, true);
//     // ASSERT_EQ(recv_called, true);
//     // ASSERT_EQ(recv1_called, true);
// }
