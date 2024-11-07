// Utilities used across all files in project.
#ifndef ZAD2_UTILS_H
#define ZAD2_UTILS_H

#include <iostream>
#include <boost/asio.hpp>
#include "buffer.h"

namespace asio = boost::asio;
using port_t = uint16_t;
using send_buffer_ptr = std::shared_ptr<SendBuffer>;
using recv_buffer_ptr = std::shared_ptr<ReceiveBuffer>;

// Used when error which terminates program occurs.
class TerminatingError {
public:
    template<typename ...Args>
    static void fatal(Args &&...args) {
        (std::cerr << ... << args);
        std::cerr << std::endl;
        exit(1);
    }
};

// Checks if debug option is present in compilation.
class Debug {
private:
#ifdef NDEBUG
    static constexpr bool debug = false;
#else
    static constexpr bool debug = true;
#endif

public:
    template<typename ...Args>
    static void log(Args &&...args) {
        if (debug) {
            (std::cerr << ... << args);
            std::cerr << std::endl;
        }
    }
};

// Represents ip address + port.
class Address {
private:
    std::string address;
    std::string ip, port;

public:
    explicit Address(const std::string &address_s);

    std::string get_address();

    std::string get_ip();

    std::string get_port();
};

// Serialization of vector of serializable items.
template<typename T>
void serialize_vector(const send_buffer_ptr &buffer, const std::vector<T> &vec) {
    auto size = (list_size_t) vec.size();
    buffer->copy_into(size);
    for (auto &item : vec)
        item->serialize(buffer);
}

// Serialization of map <id, serializable_item>.
template<typename T1, typename T2>
void serialize_map(const send_buffer_ptr &buffer, const std::map<T1, T2> &mp) {
    auto size = (map_size_t) mp.size();
    buffer->copy_into(size);
    for (auto &[key, value] : mp) {
        buffer->copy_into(key);
        value->serialize(buffer);
    }
}

// Serialization values of map <id, serializable_item>.
template<typename T1, typename T2>
void serialize_map_keys(const send_buffer_ptr &buffer, const std::map<T1, T2> &mp) {
    auto size = (list_size_t) mp.size();
    buffer->copy_into(size);
    for (auto &[key, value] : mp)
        value->serialize(buffer);
}

#endif //ZAD2_UTILS_H
