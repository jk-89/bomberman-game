#include "input_parser.h"
#include "random_generator.h"

ServerOptions::ServerOptions(int argc, char **argv) : description("Allowed options") {
    description.add_options()
            ("bomb-timer,b", p_options::value<uint16_t>(&bomb_timer)->required())
            ("players-count,c", p_options::value<uint16_t>(&players_count)->required())
            ("turn-duration,d", p_options::value<uint64_t>(&turn_duration)->required())
            ("explosion-radius,e", p_options::value<uint16_t>(&explosion_radius)->required())
            ("help,h", "shows help message")
            ("initial-blocks,k", p_options::value<uint16_t>(&initial_blocks)->required())
            ("game-length,l", p_options::value<uint16_t>(&game_length)->required())
            ("server-name,n", p_options::value<std::string>(&server_name)->required())
            ("port,p", p_options::value<port_t>(&port)->required(),
             "port used by server to receive communicates from clients")
            ("seed,s", p_options::value<uint32_t>(&seed))
            ("size-x,x", p_options::value<uint16_t>(&size_x)->required())
            ("size-y,y", p_options::value<uint16_t>(&size_y)->required());

    p_options::store(p_options::parse_command_line(argc, argv, description), vm);

    if (vm.contains("help")) {
        std::cout << description << std::endl;
        exit(0);
    }

    p_options::notify(vm);
    if (players_count > UINT8_MAX) {
        Debug::log("Improper players count value.");
        throw std::exception();
    }
}

port_t ServerOptions::get_port() {
    return port;
}

void ServerOptions::initialize_game_state(std::shared_ptr<GameInfo> &game_info,
                                          const std::shared_ptr<asio::io_context> &io_context) {
    std::shared_ptr<RandomGenerator> rng;
    if (vm.contains("seed"))
        rng = std::make_shared<RandomGenerator>(seed);
    else
        rng = std::make_shared<RandomGenerator>();
    game_info = std::make_shared<GameInfo>
            (server_name, (uint8_t) players_count, size_x, size_y, game_length,
             explosion_radius, bomb_timer, turn_duration, initial_blocks, rng,
             io_context);
}
