#include "move_message.h"

Join::Join(const std::string &name) : name(name) {}

void Join::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
    buffer->copy_string_into(name);
}

void PlaceBomb::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
}

void PlaceBlock::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
}

void Move::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(ID);
    buffer->copy_into((direction_t) direction);
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
