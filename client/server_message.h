// Types of messages which server sends.
// Change_info() functions are used to properly change state of game.
#ifndef ZAD2_SERVER_MESSAGE_H
#define ZAD2_SERVER_MESSAGE_H

#include "utils.h"
#include "map_objects.h"
#include "player.h"
#include "event.h"
#include "game_state.h"

class Hello {
private:
    std::string server_name{};
    uint8_t player_count{};
    coordinate_t size_x{}, size_y{};
    uint16_t game_length{};
    uint16_t explosion_radius{};
    uint16_t bomb_timer{};

public:
    constexpr static uint8_t ID = 0;
    void deserialize(const recv_buffer_ptr &buffer);

    void change_info(GameInfo &game_info);
};

class AcceptedPlayer {
private:
    Player::player_id_t player_id{};
    std::shared_ptr<Player> player{};

public:
    constexpr static uint8_t ID = 1;
    void deserialize(const recv_buffer_ptr &buffer);
    void change_info(GameInfo &game_info);
};

class GameStarted {
private:
    Player::player_map_t player_map{};

public:
    constexpr static uint8_t ID = 2;
    void deserialize(const recv_buffer_ptr &buffer);
    void change_info(GameInfo &game_info);
};

class Turn {
private:
    uint16_t turn_no{};
    EventVariant events{};
    void change_info_explosion(GameInfo &game_info, BombExploded &bomb_exploded,
                               std::set<Player::player_id_t> &dead_players);

public:
    constexpr static uint8_t ID = 3;
    void deserialize(const recv_buffer_ptr &buffer);
    void change_info(GameInfo &game_info);
};

class GameEnded {
private:
    std::map<Player::player_id_t, Score::score_t> scores{};

public:
    constexpr static uint8_t ID = 4;
    void deserialize(const recv_buffer_ptr &buffer);
};

#endif //ZAD2_SERVER_MESSAGE_H
