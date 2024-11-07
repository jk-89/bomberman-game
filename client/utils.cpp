#include <iostream>
#include "utils.h"

Address::Address(const std::string &address_s) {
    // Parsing input string to address and port.
    address = address_s;
    static std::string delimiter = ":";
    size_t position = address.find_last_of(delimiter);
    if (position == std::string::npos)
        throw std::invalid_argument("Invalid ip address.");
    ip = address.substr(0, position);
    port = address.substr(position + 1, std::string::npos);
}

std::string Address::get_address() {
    return address;
}

std::string Address::get_ip() {
    return ip;
}

std::string Address::get_port() {
    return port;
}
