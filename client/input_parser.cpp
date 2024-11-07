#include "input_parser.h"

ProgramOptions::ProgramOptions(int argc, char **argv) : description("Allowed options") {
    input_address_t gui_addr, server_addr;
    description.add_options()
            ("gui-address,d", p_options::value<input_address_t>(&gui_addr)->required(),
             "<(hostname):(port) or (IPv4):(port) or (IPv6):(port)>")
            ("help,h", "shows help message")
            ("player-name,n", p_options::value<std::string>(&name)->required(),
             "name of player")
            ("port,p", p_options::value<port_t>(&port)->required(),
             "port used by client to communicate with GUI")
            ("server-address,s", p_options::value<input_address_t>(&server_addr)->required(),
             "<(hostname):(port) or (IPv4):(port) or (IPv6):(port)>");
    p_options::store(p_options::parse_command_line(argc, argv, description), vm);
    
    if (vm.contains("help")) {
        std::cout << description << std::endl;
        exit(0);
    }

    p_options::notify(vm);
    gui_address = std::make_shared<Address>(gui_addr);
    server_address = std::make_shared<Address>(server_addr);
}

const std::shared_ptr<Address> &ProgramOptions::get_gui_address() const {
    return gui_address;
}

const std::shared_ptr<Address> &ProgramOptions::get_server_address() const {
    return server_address;
}

port_t ProgramOptions::get_port() {
    return port;
}

std::string ProgramOptions::get_name() {
    return name;
}
