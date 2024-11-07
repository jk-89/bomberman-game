// Contains info about game. Used while processing moves
// and sending information to client.

#ifndef ZAD2_GAME_STATE_H
#define ZAD2_GAME_STATE_H

#include <set>
#include "utils.h"
#include "player.h"
#include "map_objects.h"
#include "event.h"
#include "move.h"
#include "random_generator.h"
#include "client_connection.h"

// Contains all needed information about game.
class GameInfo {
private:
    constexpr static uint8_t HELLO_ID = 0;
    constexpr static uint8_t ACCEPTED_PLAYER_ID = 1;
    constexpr static uint8_t GAME_STARTED_ID = 2;
    constexpr static uint8_t TURN_ID = 3;
    constexpr static uint8_t GAME_ENDED_ID = 4;

    // ID generated after connection.
    Player::player_id_t connected_ids{};
    // ID generated after join.
    Player::player_id_t curr_player_id{};
    Bomb::bomb_id_t curr_bomb_id{};
    std::string server_name{};
    uint8_t player_count{};
    coordinate_t size_x, size_y{};
    uint16_t game_length{};
    uint16_t explosion_radius{};
    uint16_t bomb_timer{};
    uint64_t turn_duration{};
    uint16_t initial_blocks{};
    uint16_t turn_no{};
    std::shared_ptr<RandomGenerator> rng;
    std::shared_ptr<asio::io_context> io_context;
    asio::steady_timer timer;
    std::map<Player::player_id_t, std::shared_ptr<Player>> players{};
    // Player or spectators who did not disconnect yet.
    std::map<Player::player_id_t, std::shared_ptr<Connection>> active_users{};
    // Maps initial_id to game_id.
    std::map<Player::player_id_t, Player::player_id_t> ids_map{};
    std::map<Player::player_id_t, Position> player_positions{};
    std::vector<Position> blocks{};
    std::map<Bomb::bomb_id_t, std::shared_ptr<Bomb>> bombs{};
    std::map<Player::player_id_t, std::shared_ptr<Score>> scores{};
    std::vector<send_buffer_ptr> past_turns{};

    // Players who died during turn.
    std::set<Player::player_id_t> dead_players{};
    // Unblocking tiles after explosions.
    std::vector<Position> tiles_to_unblock{};
    // Bombs which exploded during turn.
    std::vector<Bomb::bomb_id_t> exploded_bombs{};
    // Moves which players will perform during turn.
    MoveVariant moves{};

    // Checks if game has started or is in lobby state.
    bool game_has_started();

    void serialize_hello(const send_buffer_ptr &buffer);

    void serialize_accepted_player(const send_buffer_ptr &buffer, uint8_t id,
                                   const std::shared_ptr<Player> &player);

    void serialize_game_started(const send_buffer_ptr &buffer);

    void serialize_turn(const send_buffer_ptr &buffer, EventVariant &events);

    void serialize_game_ended(const send_buffer_ptr &buffer);

    // Reset previous game data.
    void reset();

    // New bomb placed.
    void place_bomb(Bomb::bomb_id_t id, const std::shared_ptr<Bomb> &bomb);

    // New position blocked.
    void block_tile(Position position);

    // Checks if position is blocked. If it is then add information about
    // destroying it with bomb.
    bool is_blocked(coordinate_t x, coordinate_t y, BombExploded &bomb_exploded);

    // Checks if position is blocked.
    bool tile_blocked(coordinate_t x, coordinate_t y);

    // New player position.
    void move_player(Player::player_id_t player_id, Position position);

    // Player score changes.
    void update_score(Player::player_id_t player_id);

    // Explosion may destroy some robots.
    void destroy_robots(coordinate_t x, coordinate_t y, BombExploded &bomb_exploded);

    // Bomb timers decrease each turn.
    void decrease_bombs_timer(EventVariant &events);

    // Position isn't blocked anymore.
    void unblock_tile(Position position);

    // Bomb exploded and should be deleted.
    void bomb_exploded(Bomb::bomb_id_t id);

    // Processing single move.
    void make_single_move(EventVariant &events, Player::player_id_t player_id,
                          MoveVariant::move_variant_t &move);

    // Processing players' moves.
    void make_moves(EventVariant &events);

    // Iterate over players' moves and generate proper events.
    void proceed_turn(EventVariant &events);

    // Sends turn serialization to all connected clients.
    void send_curr_turn(const send_buffer_ptr &buffer);

    // Handling initial turn in a proper way.
    void process_turn0();

    // Start timer for turn.
    void process_game_timer();

    // Process turn.
    void process_game();

public:
    GameInfo(const std::string &server_name, uint8_t player_count, coordinate_t size_x,
             coordinate_t size_y, uint16_t game_length, uint16_t explosion_radius,
             uint16_t bomb_timer, uint64_t turn_duration, uint16_t initial_blocks,
             const std::shared_ptr<RandomGenerator> &rng,
             const std::shared_ptr<asio::io_context> &io_context_);

    // Adding new client to list, sending Hello.
    void new_connection(asio::ip::tcp::socket socket);

    // Erasing client from list.
    void delete_connection(Player::player_id_t init_id);

    // If there are free slots then generates id for new player
    // and creates AcceptedPlayer.
    void accept_player(const std::shared_ptr<Player> &player,
                       Player::player_id_t init_id);

    // Insert move which should be processed during the turn.
    void insert_move(Player::player_id_t init_id, MoveVariant::move_variant_t &move);;
};

#endif //ZAD2_GAME_STATE_H
