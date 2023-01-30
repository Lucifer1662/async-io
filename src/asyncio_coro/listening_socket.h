#pragma once

class ListeningSocket : public RawSocket, SocketContextHandler {
    SocketContext &context;

  public:
    ListeningSocket(SocketContext &context, RawSocket socket);
    ListeningSocket(const ListeningSocket &) = delete;
    ListeningSocket(ListeningSocket &&) = delete;
    static std::optional<ListeningSocket> create(SocketContext &context);

    static std::optional<std::unique_ptr<ListeningSocket>> create_ptr(SocketContext &context);

    void on_read(SocketContext &context, int i) override { read_available(); }

    ~ListeningSocket();
};
