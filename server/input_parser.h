// Used to parse input from user from terminal.

#ifndef ZAD2_INPUT_PARSER_H
#define ZAD2_INPUT_PARSER_H

#include <boost/program_options.hpp>
#include "utils.h"
#include "game_state.h"

namespace p_options = boost::program_options;

class ServerOptions {
private:
    p_options::options_description description;
    p_options::variables_map vm;
    uint16_t bomb_timer{};
    uint16_t players_count{};
    uint64_t turn_duration{};
    uint16_t explosion_radius{};
    uint16_t initial_blocks{};
    uint16_t game_length{};
    std::string server_name;
    port_t port{};
    uint32_t seed{};
    uint16_t size_x{}, size_y{};

public:
    // Constructor takes as input all params passed by user in command line.
    // Then parses data and saves it.
    ServerOptions(int argc, char **argv);

    port_t get_port();

    void initialize_game_state(std::shared_ptr<GameInfo> &game_info,
                               const std::shared_ptr<asio::io_context> &io_context);
};


#endif //ZAD2_INPUT_PARSER_H
