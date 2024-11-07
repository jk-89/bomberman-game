#include "event.h"

void BombPlaced::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_from(bomb_id);
    position.deserialize(buffer);
}

void BombExploded::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_from(bomb_id);
    list_size_t size;
    buffer->copy_from(size);
    for (list_size_t i = 0; i < size; i++) {
        Player::player_id_t player_id;
        buffer->copy_from(player_id);
        robots_destroyed.emplace_back(player_id);
    }
    buffer->copy_from(size);
    for (list_size_t i = 0; i < size; i++) {
        Position position;
        position.deserialize(buffer);
        blocks_destroyed.emplace_back(position);
    }
}

void PlayerMoved::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_from(player_id);
    position.deserialize(buffer);
}

void BlockPlaced::deserialize(const recv_buffer_ptr &buffer) {
    position.deserialize(buffer);
}

void EventVariant::insert_event(EventVariant::variant_t &event) {
    events.emplace_back(event);
}
