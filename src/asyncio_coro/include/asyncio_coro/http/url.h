#pragma once
#include <string>
#include <unordered_map>
#include <stdint.h>

namespace Http {

struct Protocol {
    enum ProtocolEnum { HTTP, HTTPS };

  private:
    ProtocolEnum mProtocol;

  public:
    Protocol(ProtocolEnum protocol)
        : mProtocol(protocol) {}
    Protocol() = default;
    Protocol(const Protocol &) = default;
    Protocol(Protocol &&) = default;
    Protocol &operator=(const Protocol &) = default;

    bool operator==(const Protocol &m) { return m.mProtocol == mProtocol; }
    bool operator!=(const Protocol &m) { return m.mProtocol != mProtocol; }

    std::string to_string() const {
        switch (mProtocol) {
        case HTTP:
            return "http";
        case HTTPS:
            return "https";
        }
        return "";
    }

    std::string to_request_line_string() const {
        switch (mProtocol) {
        case HTTP:
            return "HTTP";
        case HTTPS:
            return "HTTP";
        }
        return "";
    }
};

struct URL {
    std::unordered_map<std::string, std::string> params;
    Protocol protocol = Protocol::HTTP;
    std::string host = "";
    std::string path = "";
    unsigned short port = 80;

    std::string to_full_url() const { return host + path + std::to_string(port); };
    std::string to_host_path() const {
        if (port == 80) {
            return host;
        } else {
            return host + ":" + std::to_string(port);
        }
    };

    // void set_host(std::string host) { this->host = std::move(host); }
};

class Method {
  public:
    enum MethodEnum { GET, POST, UPDATE, PATCH, OPTIONS };

  private:
    MethodEnum method;

  public:
    Method(MethodEnum method)
        : method(method) {}

    bool operator==(const Method &m) { return m.method == method; }
    bool operator!=(const Method &m) { return m.method != method; }

    std::string to_string() const {
        switch (method) {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case UPDATE:
            return "UPDATE";
        case PATCH:
            return "PATCH";
        case OPTIONS:
            return "OPTIONS";
        }
        return "";
    }
};

struct HttpVersion {
    size_t major;
    size_t minor;

    HttpVersion(size_t major, size_t minor)
        : major(major)
        , minor(minor) {}

    std::string to_string() const {
        return minor == 0 ? std::to_string(major) : std::to_string(major) + "." + std::to_string(minor);
    }
};

struct RequestLine {
    Method method;
    std::string request_uri;
    Protocol protocol;
    HttpVersion version;

    RequestLine(Method method, std::string request_uri, Protocol protocol, HttpVersion version)
        : method(method)
        , request_uri(request_uri)
        , protocol(protocol)
        , version(version) {}

    RequestLine() = default;
    RequestLine(const RequestLine &) = default;
    RequestLine(RequestLine &&) = default;
    RequestLine &operator=(const RequestLine &) = default;

    std::string to_string() const {
        return method.to_string() + " " + request_uri + " " + protocol.to_request_line_string() + "/" +
            version.to_string() + "\r\n";
    }
};

}   // namespace Http