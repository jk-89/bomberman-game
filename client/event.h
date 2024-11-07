// Events present in Turns.
#ifndef ZAD2_EVENT_H
#define ZAD2_EVENT_H

#include <variant>
#include "utils.h"
#include "map_objects.h"
#include "player.h"

class BombPlaced {
private:
    friend class Turn;
    Bomb::bomb_id_t bomb_id{};
    Position position{};

public:
    constexpr static uint8_t ID = 0;
    void deserialize(const recv_buffer_ptr &buffer);
};

class BombExploded {
private:
    friend class Turn;
    Bomb::bomb_id_t bomb_id{};
    std::vector<Player::player_id_t> robots_destroyed{};
    std::vector<Position> blocks_destroyed{};

public:
    constexpr static uint8_t ID = 1;
    void deserialize(const recv_buffer_ptr &buffer);
};

class PlayerMoved {
private:
    friend class Turn;
    Player::player_id_t player_id{};
    Position position{};

public:
    constexpr static uint8_t ID = 2;
    void deserialize(const recv_buffer_ptr &buffer);
};

class BlockPlaced {
private:
    friend class Turn;
    Position position{};

public:
    constexpr static uint8_t ID = 3;
    void deserialize(const recv_buffer_ptr &buffer);
};

// Variant of all possible events.
class EventVariant {
public:
    using variant_t = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

private:
    friend class Turn;
    std::vector<variant_t> events{};

public:
    void insert_event(variant_t &event);
};

#endif //ZAD2_EVENT_H
