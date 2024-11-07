#include "event.h"

BombPlaced::BombPlaced(Bomb::bomb_id_t bomb_id, const Position &position) :
        bomb_id(bomb_id), position(position) {}

void BombPlaced::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
    buffer->copy_into(bomb_id);
    position.serialize(buffer);
}


BombExploded::BombExploded(Bomb::bomb_id_t bomb_id) : bomb_id(bomb_id) {}

void BombExploded::insert_destroyed_robot(Player::player_id_t id) {
    robots_destroyed.emplace_back(id);
}

void BombExploded::insert_destroyed_block(Position position) {
    blocks_destroyed.emplace_back(position);
}

void BombExploded::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
    buffer->copy_into(bomb_id);

    auto size = (list_size_t) robots_destroyed.size();
    buffer->copy_into(size);
    for (auto &player_id: robots_destroyed)
        buffer->copy_into(player_id);

    size = (list_size_t) blocks_destroyed.size();
    buffer->copy_into(size);
    for (auto &position: blocks_destroyed)
        position.serialize(buffer);
}


void PlayerMoved::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
    buffer->copy_into(player_id);
    position.serialize(buffer);
}

PlayerMoved::PlayerMoved(Player::player_id_t id, const Position &position)
        : player_id(id), position(position) {}


BlockPlaced::BlockPlaced(const Position &position) : position(position) {}

void BlockPlaced::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
    position.serialize(buffer);
}


void EventVariant::insert_event(EventVariant::variant_t &event) {
    events.emplace_back(event);
}

void EventVariant::serialize(const send_buffer_ptr &buffer) {
    auto size = (list_size_t) events.size();
    buffer->copy_into(size);
    for (auto &event: events) {
        if (std::holds_alternative<BombPlaced>(event)) {
            BombPlaced bomb_placed = std::get<BombPlaced>(event);
            bomb_placed.serialize(buffer);
        } else if (std::holds_alternative<BombExploded>(event)) {
            BombExploded bomb_exploded = std::get<BombExploded>(event);
            bomb_exploded.serialize(buffer);
        } else if (std::holds_alternative<PlayerMoved>(event)) {
            PlayerMoved player_moved = std::get<PlayerMoved>(event);
            player_moved.serialize(buffer);
        } else if (std::holds_alternative<BlockPlaced>(event)) {
            BlockPlaced block_placed = std::get<BlockPlaced>(event);
            block_placed.serialize(buffer);
        }
    }
}
