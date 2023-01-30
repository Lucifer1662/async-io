#pragma once

#include <optional>
#include <vector>
#include <string>

struct IPAddress {
    bool is_ipv4;
    std::vector<unsigned char> address;
    unsigned short port;
    bool is_loop_back;

    IPAddress(bool is_ipv4, std::vector<unsigned char> address, int port, bool is_loop_back)
        : is_ipv4(is_ipv4)
        , address(std::move(address))
        , port(port)
        , is_loop_back(is_loop_back) {}

    std::string ip_to_string() const;
    static std::optional<IPAddress> from_string(const std::string &ip, int port = -1);
    static IPAddress loopback(int port) { return IPAddress(true, {}, port, true); }
};

// host_name: for example www.google.com
// service_name: for example 8080 or http address
std::vector<IPAddress> IPAddress_from_URL(const char *host_name, const char *service_name);
;