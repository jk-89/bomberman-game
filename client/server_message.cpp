#include <iostream>
#include <boost/array.hpp>
#include "server_message.h"
#include "utils.h"

void Hello::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_string_from(server_name);
    buffer->copy_from(player_count);
    buffer->copy_from(size_x);
    buffer->copy_from(size_y);
    buffer->copy_from(game_length);
    buffer->copy_from(explosion_radius);
    buffer->copy_from(bomb_timer);
}

void Hello::change_info(GameInfo &game_info) {
    game_info.server_name = server_name;
    game_info.player_count = player_count;
    game_info.size_x = size_x;
    game_info.size_y = size_y;
    game_info.game_length = game_length;
    game_info.explosion_radius = explosion_radius;
    game_info.bomb_timer = bomb_timer;
}

void AcceptedPlayer::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_from(player_id);
    player = std::make_shared<Player>();
    player->deserialize(buffer);
}

void AcceptedPlayer::change_info(GameInfo &game_info) {
    game_info.players.emplace(player_id, player);
}

void GameStarted::deserialize(const recv_buffer_ptr &buffer) {
    map_size_t size;
    buffer->copy_from(size);
    for (map_size_t i = 0; i < size; i++) {
        Player::player_id_t player_id;
        std::shared_ptr<Player> player = std::make_shared<Player>();

        buffer->copy_from(player_id);
        player->deserialize(buffer);
        player_map.emplace(player_id, player);
    }
}

// Refill players, fill scores.
void GameStarted::change_info(GameInfo &game_info) {
    game_info.players.clear();
    for (auto &[player_id, player]: player_map) {
        game_info.players.emplace(player_id, player);
        game_info.scores.emplace(player_id, std::make_shared<Score>());
    }
    game_info.set_game();
}

void Turn::deserialize(const recv_buffer_ptr &buffer) {
    buffer->copy_from(turn_no);
    list_size_t size;
    buffer->copy_from(size);
    for (map_size_t i = 0; i < size; i++) {
        uint8_t event_id;
        buffer->copy_from(event_id);
        EventVariant::variant_t new_event;
        if (event_id == BombPlaced::ID) {
            BombPlaced bomb_placed;
            bomb_placed.deserialize(buffer);
            new_event = bomb_placed;
        } else if (event_id == BombExploded::ID) {
            BombExploded bomb_exploded;
            bomb_exploded.deserialize(buffer);
            new_event = bomb_exploded;
        } else if (event_id == PlayerMoved::ID) {
            PlayerMoved player_moved;
            player_moved.deserialize(buffer);
            new_event = player_moved;
        } else if (event_id == BlockPlaced::ID) {
            BlockPlaced block_placed;
            block_placed.deserialize(buffer);
            new_event = block_placed;
        } else {
            throw WrongMessage();
        }
        events.insert_event(new_event);
    }
}

// Modify killed players, erase bomb, save affected tiles.
void Turn::change_info_explosion(GameInfo &game_info, BombExploded &bomb_exploded,
                                 std::set<Player::player_id_t> &dead_players) {
    for (auto &player_id: bomb_exploded.robots_destroyed) {
        // Player wasn't kill this turn yet.
        if (dead_players.find(player_id) == dead_players.end()) {
            dead_players.emplace(player_id);
            game_info.update_score(player_id);
        }
    }

    auto bomb = game_info.bombs.at(bomb_exploded.bomb_id);
    auto bomb_pos = bomb->get_position();
    auto x = bomb_pos.get_x();
    auto y = bomb_pos.get_y();
    game_info.explosions_unique.emplace(x, y);

    game_info.bomb_exploded(bomb_exploded.bomb_id);
    for (auto &position: bomb_exploded.blocks_destroyed)
        game_info.tiles_to_unblock.emplace_back(position);

    if (game_info.is_blocked(x, y))
        return;
    for (coordinate_t i = 1; i <= game_info.explosion_radius && x >= i; i++) {
        game_info.explosions_unique.emplace(x - i, y);
        if (game_info.is_blocked(x - i, y))
            break;
    }
    for (coordinate_t i = 1; i <= game_info.explosion_radius && y >= i; i++) {
        game_info.explosions_unique.emplace(x, y - i);
        if (game_info.is_blocked(x, y - i))
            break;
    }
    for (coordinate_t i = 1; i <= game_info.explosion_radius && x < game_info.size_x - i; i++) {
        game_info.explosions_unique.emplace(x + i, y);
        if (game_info.is_blocked(x + i, y))
            break;
    }
    for (coordinate_t i = 1; i <= game_info.explosion_radius && y < game_info.size_y - i; i++) {
        game_info.explosions_unique.emplace(x, y + i);
        if (game_info.is_blocked(x, y + i))
            break;
    }
}

void Turn::change_info(GameInfo &game_info) {
    game_info.turn = turn_no;
    game_info.explosions.clear();
    game_info.explosions_unique.clear();
    game_info.tiles_to_unblock.clear();
    std::set<Player::player_id_t> dead_players;
    game_info.decrease_bombs_timer();
    for (auto &event: events.events) {
        if (std::holds_alternative<BombPlaced>(event)) {
            BombPlaced bomb_placed = std::get<BombPlaced>(event);
            std::shared_ptr<Bomb> bomb =
                    std::make_shared<Bomb>(bomb_placed.position, game_info.bomb_timer);
            game_info.place_bomb(bomb_placed.bomb_id, bomb);
        } else if (std::holds_alternative<BombExploded>(event)) {
            BombExploded bomb_exploded = std::get<BombExploded>(event);
            change_info_explosion(game_info, bomb_exploded, dead_players);
        } else if (std::holds_alternative<PlayerMoved>(event)) {
            PlayerMoved player_moved = std::get<PlayerMoved>(event);
            game_info.move_player(player_moved.player_id,
                                  std::make_shared<Position>(player_moved.position));
        } else if (std::holds_alternative<BlockPlaced>(event)) {
            BlockPlaced block_placed = std::get<BlockPlaced>(event);
            game_info.block_tile(std::make_shared<Position>(block_placed.position));
        }
    }
    // Tiles and explosions are safely redescribed at the end.
    for (auto &position: game_info.tiles_to_unblock)
        game_info.unblock_tile(position);
    for (auto &coord: game_info.explosions_unique)
        game_info.explosions.emplace_back(std::make_shared<Position>(coord.first, coord.second));
}

void GameEnded::deserialize(const recv_buffer_ptr &buffer) {
    map_size_t size;
    buffer->copy_from(size);
    for (map_size_t i = 0; i < size; i++) {
        Player::player_id_t player_id;
        Score score;

        buffer->copy_from(player_id);
        score.deserialize(buffer);
        scores.emplace(player_id, score.get_score());
    }
}
