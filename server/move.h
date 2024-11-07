// Communicates used in client-server messages.
// Communicates describe moves performed by player.

#ifndef ZAD2_MOVE_H
#define ZAD2_MOVE_H

#include <variant>
#include "utils.h"
#include "map_objects.h"
#include "player.h"

class Join {
private:
    std::string name{};

public:
    constexpr static uint8_t ID = 0;
    void deserialize(const recv_buffer_ptr &buffer);

    std::string get_name();
};

class PlaceBomb {
public:
    constexpr static uint8_t ID = 1;
};

class PlaceBlock {
public:
    constexpr static uint8_t ID = 2;
};

class Move {
private:
    friend class GameInfo;
    Direction direction{};

public:
    constexpr static uint8_t ID = 3;
    void deserialize(const recv_buffer_ptr &buffer);
    uint8_t get_direction();
};

// Variant of all possible player actions.
class MoveVariant {
public:
    using move_variant_t = std::variant<Join, PlaceBomb, PlaceBlock, Move>;

private:
    friend class GameInfo;
    std::map<Player::player_id_t, move_variant_t> moves;

public:
    void insert_event(Player::player_id_t id, move_variant_t &move);
};

#endif //ZAD2_MOVE_H
