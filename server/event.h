// Events present in Turns.

#ifndef ZAD2_EVENT_H
#define ZAD2_EVENT_H

#include <variant>
#include "utils.h"
#include "map_objects.h"
#include "player.h"

class BombPlaced {
private:
    Bomb::bomb_id_t bomb_id{};
    Position position{};

public:
    constexpr static uint8_t ID = 0;

    BombPlaced(Bomb::bomb_id_t bomb_id, const Position &position);

    void serialize(const send_buffer_ptr &buffer);
};

class BombExploded {
private:
    Bomb::bomb_id_t bomb_id{};
    std::vector<Player::player_id_t> robots_destroyed{};
    std::vector<Position> blocks_destroyed{};

public:
    constexpr static uint8_t ID = 1;

    explicit BombExploded(Bomb::bomb_id_t bomb_id);

    void insert_destroyed_robot(Player::player_id_t id);

    void insert_destroyed_block(Position position);

    void serialize(const send_buffer_ptr &buffer);
};

class PlayerMoved {
private:
    Player::player_id_t player_id{};
    Position position{};

public:
    constexpr static uint8_t ID = 2;

    PlayerMoved(Player::player_id_t id, const Position &position);

    void serialize(const send_buffer_ptr &buffer);
};

class BlockPlaced {
private:
    Position position{};

public:
    constexpr static uint8_t ID = 3;

    explicit BlockPlaced(const Position &position);

    void serialize(const send_buffer_ptr &buffer);
};

// Variant of all possible events.
class EventVariant {
public:
    using variant_t = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

private:
    std::vector<variant_t> events{};

public:
    void insert_event(variant_t &event);
    void serialize(const send_buffer_ptr &buffer);
};

#endif //ZAD2_EVENT_H
