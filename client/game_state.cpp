#include "game_state.h"

void GameInfo::serialize_lobby(const send_buffer_ptr &buffer) {
    buffer->copy_string_into(server_name);
    buffer->copy_into(player_count);
    buffer->copy_into(size_x);
    buffer->copy_into(size_y);
    buffer->copy_into(game_length);
    buffer->copy_into(explosion_radius);
    buffer->copy_into(bomb_timer);
    serialize_map(buffer, players);
}

void GameInfo::serialize_game(const send_buffer_ptr &buffer) {
    buffer->copy_string_into(server_name);
    buffer->copy_into(size_x);
    buffer->copy_into(size_y);
    buffer->copy_into(game_length);
    buffer->copy_into(turn);
    serialize_map(buffer, players);
    serialize_map(buffer, player_positions);
    serialize_vector(buffer, blocks);
    serialize_map_keys(buffer, bombs);
    serialize_vector(buffer, explosions);
    serialize_map(buffer, scores);
}

void GameInfo::serialize(const send_buffer_ptr &buffer) {
    uint8_t id = state;
    buffer->copy_into(id);
    if (state == LOBBY)
        serialize_lobby(buffer);
    else
        serialize_game(buffer);
}

State GameInfo::get_state() {
    return state;
}

void GameInfo::set_game() {
    state = GAME;
}

void GameInfo::reset() {
    state = LOBBY;
    players.clear();
    player_positions.clear();
    blocks.clear();
    bombs.clear();
    explosions.clear();
    scores.clear();
}

void GameInfo::bomb_exploded(Bomb::bomb_id_t id) {
    auto iter = bombs.find(id);
    if (iter != bombs.end())
        bombs.erase(iter);
}

void GameInfo::place_bomb(Bomb::bomb_id_t id, const std::shared_ptr<Bomb> &bomb) {
    bombs.emplace(id, bomb);
}

void GameInfo::block_tile(const std::shared_ptr<Position> &position) {
    blocks.emplace_back(position);
}

void GameInfo::unblock_tile(Position position) {
    for (size_t i = 0; i < blocks.size(); i++) {
        // If we find matching position (blocked positions are unique)
        // we swap it with last element in vector and then pop it.
        if (blocks[i]->equals(position)) {
            std::swap(blocks[i], blocks.back());
            blocks.pop_back();
        }
    }
}

bool GameInfo::is_blocked(coordinate_t x, coordinate_t y) {
    Position new_pos(x, y);
    for (const auto &pos : blocks) {
        if (new_pos.equals(*pos))
            return true;
    }
    return false;
}

void GameInfo::move_player(Player::player_id_t player_id, const std::shared_ptr<Position> &position) {
    auto iter = player_positions.find(player_id);
    if (iter == player_positions.end())
        player_positions.emplace(player_id, position);
    else
        iter->second = position;
}

void GameInfo::update_score(Player::player_id_t player_id) {
    scores.at(player_id)->increase();
}

void GameInfo::decrease_bombs_timer() {
    for (auto &[_, bomb] : bombs)
        bomb->decrease_timer();
}
