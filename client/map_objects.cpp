#include "map_objects.h"

Position::Position(coordinate_t x, coordinate_t y) : x(x), y(y) {}

bool Position::equals(Position &other) {
    return x == other.x && y == other.y;
}

Position::Position() = default;

void Position::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(x);
    buffer->copy_into(y);
}

void Position::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_from(x);
    buffer->copy_from(y);
}

coordinate_t Position::get_x() {
    return x;
}

coordinate_t Position::get_y() {
    return y;
}

Bomb::Bomb(const Position &position, bomb_timer_t timer)
        : position(position), timer(timer) {}

void Bomb::serialize(const send_buffer_ptr &buffer) {
    position.serialize(buffer);
    buffer->copy_into(timer);
}

void Bomb::decrease_timer() {
    timer--;
}

Position &Bomb::get_position() {
    return position;
}
