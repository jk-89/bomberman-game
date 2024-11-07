// Contains info about game. Used while sending information to GUI and
// receiving it from server.

#ifndef ZAD2_GAME_STATE_H
#define ZAD2_GAME_STATE_H

#include <set>
#include "utils.h"
#include "player.h"
#include "map_objects.h"

enum State {
    LOBBY = 0,
    GAME = 1
};

// Contains all needed information about game.
class GameInfo {
private:
    friend class Hello;
    friend class AcceptedPlayer;
    friend class GameStarted;
    friend class Turn;
    friend class GameEnded;

    State state = LOBBY;
    std::string server_name{};
    uint8_t player_count{};
    coordinate_t size_x, size_y{};
    uint16_t game_length{};
    uint16_t explosion_radius{};
    uint16_t bomb_timer{};
    uint16_t turn{};
    std::map<Player::player_id_t, std::shared_ptr<Player>> players{};
    std::map<Player::player_id_t, std::shared_ptr<Position>> player_positions{};
    std::vector<std::shared_ptr<Position>> blocks{};
    std::map<Bomb::bomb_id_t, std::shared_ptr<Bomb>> bombs{};
    std::vector<std::shared_ptr<Position>> explosions{};
    // Not counting same tile twice in explosions.
    std::set<std::pair<coordinate_t, coordinate_t> > explosions_unique{};
    // Unblocking tiles after explosions.
    std::vector<Position> tiles_to_unblock{};
    std::map<Player::player_id_t, std::shared_ptr<Score>> scores{};

    void serialize_lobby(const send_buffer_ptr &buffer);
    void serialize_game(const send_buffer_ptr &buffer);

public:
    void serialize(const send_buffer_ptr &buffer);

    State get_state();

    void set_game();

    void reset();

    // Bomb exploded and should be deleted.
    void bomb_exploded(Bomb::bomb_id_t id);

    // New bomb placed.
    void place_bomb(Bomb::bomb_id_t id, const std::shared_ptr<Bomb> &bomb);

    // New position blocked.
    void block_tile(const std::shared_ptr<Position> &position);

    // Position isn't blocked anymore.
    void unblock_tile(Position position);

    // Checks if position is blocked.
    bool is_blocked(coordinate_t x, coordinate_t y);

    // New player position.
    void move_player(Player::player_id_t player_id, const std::shared_ptr<Position> &position);

    // Player score changes.
    void update_score(Player::player_id_t player_id);

    // Bomb timers deacrease each turn.
    void decrease_bombs_timer();
};

#endif //ZAD2_GAME_STATE_H
