#include "player.h"

void Score::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_into(score);
}

void Score::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_from(score);
}

Score::score_t Score::get_score() {
    return score;
}

void Score::increase() {
    score++;
}

void Player::serialize(const send_buffer_ptr &buffer) {
    buffer->copy_string_into(name);
    buffer->copy_string_into(address);
}

void Player::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_string_from(name);
    buffer->copy_string_from(address);
}