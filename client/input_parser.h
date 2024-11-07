// Used to parse input from user from terminal.

#ifndef ZAD2_INPUT_PARSER_H
#define ZAD2_INPUT_PARSER_H

#include <iostream>
#include <boost/program_options.hpp>
#include "utils.h"

namespace p_options = boost::program_options;

class ProgramOptions {
private:
    using input_address_t = std::string;

    p_options::options_description description;
    p_options::variables_map vm;
    port_t port;
    std::string name;
    std::shared_ptr<Address> gui_address, server_address;

public:
    // Constructor takes as input all params passed by user in command line.
    // Then parses data and saves it.
    ProgramOptions(int argc, char **argv);

    const std::shared_ptr<Address> &get_gui_address() const;

    const std::shared_ptr<Address> &get_server_address() const;

    port_t get_port();

    std::string get_name();
};


#endif //ZAD2_INPUT_PARSER_H
