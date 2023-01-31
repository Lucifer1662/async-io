#include "IpAddress.h"
#include "os_socket.h"

static bool get_num(const char *&s, int &num) {
    auto ss = s;
    auto i = 0;
    for (; *ss != '.' && *ss != ':' && *ss != 0; i++) {
    }

    char *endptr;
    char temp = *ss;
    *(char *)ss = 0;
    int result = strtol(s, &endptr, 10);
    if (*endptr != 0) {
        return true;
    }
    *(char *)ss = temp;

    s = ss + 1;

    num = result;

    return false;
}

std::optional<IPAddress> IPAddress::from_string(const std::string &ip, int port) {
    if (std::string::npos != ip.find("localhost")) {
        return {{true, {}, port, true}};
    }
    int c;
    std::vector<unsigned char> address;
    const char *s = ip.c_str();
    for (size_t i = 0; i < 4; i++) {
        if (get_num(s, c)) {
            address.push_back(c);
        } else {
            return {};
        }
    }

    if (port < 0) {
        if (get_num(s, c)) {
            port = c;
        }
    }

    return {{true, std::move(address), port, false}};
}

std::string IPAddress::ip_to_string() const {
    if (is_loop_back) {
        return "127.0.0.1";
    }

    std::string str;
    for (size_t i = 0; i < address.size(); i++) {
        str += std::to_string(address[i]);
        if (i + 1 != address.size()) {
            str += '.';
        }
    }
    return str;
}

#ifdef WIN32

std::vector<IPAddress> IPAddress_from_URL(const char *host_name, const char *service_name) {
    INT iRetval;
    DWORD dwRetval;

    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    struct sockaddr_in *sockaddr_ipv4;
    //    struct sockaddr_in6 *sockaddr_ipv6;
    LPSOCKADDR sockaddr_ip;

    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    dwRetval = getaddrinfo(host_name, service_name, &hints, &result);
    if (dwRetval != 0) {
        return {};
    }

    std::vector<IPAddress> addresses;

    // Retrieve each address and print out the hex bytes
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        switch (ptr->ai_family) {
        case AF_UNSPEC:
            break;
        case AF_INET:
            sockaddr_ipv4 = (struct sockaddr_in *)ptr->ai_addr;
            addresses.push_back(
                IPAddress(true,
                          std::vector<unsigned char>(
                              {sockaddr_ipv4->sin_addr.S_un.S_un_b.s_b1, sockaddr_ipv4->sin_addr.S_un.S_un_b.s_b2,
                               sockaddr_ipv4->sin_addr.S_un.S_un_b.s_b3, sockaddr_ipv4->sin_addr.S_un.S_un_b.s_b4}),
                          ntohs(sockaddr_ipv4->sin_port), false));
            break;
        case AF_INET6:
            sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;

            ipbufferlength = 46;
            iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL, ipstringbuffer, &ipbufferlength);
            if (iRetval) {
                // fail
            } else {
                // TODO handle IPV6 ipstringbuffer
            }

            break;
        case AF_NETBIOS:
            break;
        default:
            break;
        }

        // printf("\tProtocol: ");
        // switch (ptr->ai_protocol) {
        // case 0:
        //     printf("Unspecified\n");
        //     break;
        // case IPPROTO_TCP:
        //     printf("IPPROTO_TCP (TCP)\n");
        //     break;
        // case IPPROTO_UDP:
        //     printf("IPPROTO_UDP (UDP) \n");
        //     break;
        // default:
        //     printf("Other %ld\n", ptr->ai_protocol);
        //     break;
        // }

        return addresses;
    }

    freeaddrinfo(result);
}

#else
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
std::vector<IPAddress> IPAddress_from_URL(const char *host_name, const char *service_name) {
    struct hostent *he = gethostbyname("www.stackoverflow.com");
    std::vector<IPAddress> addresses;
    for (size_t i = 0; i < he->h_length; i++) {
        char *ip = inet_ntoa(*(struct in_addr *)he->h_addr_list[i]);

        addresses.push_back(IPAddress(
            true, {(unsigned char)ip[0], (unsigned char)ip[1], (unsigned char)ip[2], (unsigned char)ip[3]}, 80, false));
    }
    return addresses;
}

#endif
