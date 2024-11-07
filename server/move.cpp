#include "move.h"

void Join::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_string_from(name);
}

std::string Join::get_name() {
    return name;
}

void Move::deserialize(const recv_buffer_ptr &buffer) {
    direction_t new_direction;
    buffer->copy_from(new_direction);
    if (new_direction == Direction::UP)
        direction = UP;
    else if (new_direction == Direction::RIGHT)
        direction = RIGHT;
    else if (new_direction == Direction::DOWN)
        direction = DOWN;
    else if (new_direction == Direction::LEFT)
        direction = LEFT;
    else
        throw WrongMessage();
}

uint8_t Move::get_direction() {
    return static_cast<uint8_t>(direction);
}

void MoveVariant::insert_event(Player::player_id_t id, move_variant_t &move) {
    if (moves.contains(id))
        moves.at(id) = move;
    else
        moves.emplace(id, move);
}
