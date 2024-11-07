#include "player.h"

void Score::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(score);
}

void Score::increase() {
    score++;
}

Player::Player(const std::string &name, const std::string &address)
        : name(name), address(address) {}

void Player::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_string_into(name);
    buffer->copy_string_into(address);
}
