#include "IpAddress.h"
#include "os_socket.h"

IPAddress::IPAddress(bool is_ipv4, std::vector<unsigned char> address, int port, bool is_loop_back)
    : is_ipv4(is_ipv4)
    , address(std::move(address))
    , port(port)
    , is_loop_back(is_loop_back) {}

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
    }

    freeaddrinfo(result);
    return addresses;
}

#else
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>

bool isValidIpAddress(const char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

std::vector<IPAddress> IPAddress_from_URL(const char *host_name, const char *service_name) {

    int iRetval;

    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    struct sockaddr_in *sockaddr_ipv4;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    auto dwRetval = getaddrinfo(host_name, service_name, &hints, &result);
    if (dwRetval != 0) {
        return {};
    }

    std::vector<IPAddress> addresses;

    // Retrieve each address and print out the hex bytes
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        switch (ptr->ai_family) {
        case AF_INET: {
            sockaddr_ipv4 = (sockaddr_in *)ptr->ai_addr;
            auto ip_ptr = (unsigned char *)&sockaddr_ipv4->sin_addr;
            char str[50];
            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), (char *)&str, INET_ADDRSTRLEN);

            addresses.push_back(IPAddress(true,
                                          std::vector<unsigned char>({ip_ptr[0], ip_ptr[1], ip_ptr[2], ip_ptr[3]}),
                                          ntohs(sockaddr_ipv4->sin_port), false));
            break;
        }

        default:
            break;
        }
    }

    freeaddrinfo(result);

    return addresses;
}

// std::vector<IPAddress> IPAddress_from_URL(const char *host_name, const char *service_name) {

//     struct addrinfo hints;
//     struct addrinfo *result, *rp;
//     int sfd, s;
//     size_t len;
//     ssize_t nread;

//     /* Obtain address(es) matching host/port. */

//     std::memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
//     hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
//     hints.ai_flags = 0;
//     hints.ai_protocol = 0; /* Any protocol */

//     s = getaddrinfo(host_name, service_name, &hints, &result);
//     if (s != 0) {
//         return {};
//     }

//     std::vector<IPAddress> addresses;

//     for (rp = result; rp != NULL; rp = rp->ai_next) {
//         switch (rp->ai_protocol) {

//         case AF_INET:
//             break;
//         }

//         sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
//         if (sfd == -1)
//             continue;

//         if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
//             break; /* Success */

//         close(sfd);
//     }

//     freeaddrinfo(result);

//     std::string str = std::string(service_name) + "://" + host_name;
//     struct hostent *he = gethostbyname(str.c_str());
//     if (he == nullptr)
//         return {};
//     std::vector<IPAddress> addresses;
//     for (size_t i = 0; i < he->h_length; i++) {
//         char *ip = inet_ntoa(*(struct in_addr *)he->h_addr_list[i]);

//         addresses.push_back(IPAddress(
//             true, {(unsigned char)ip[0], (unsigned char)ip[1], (unsigned char)ip[2], (unsigned char)ip[3]}, 80,
//             false));
//     }
//     return addresses;
// }

#endif
