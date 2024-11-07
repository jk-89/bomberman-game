#include <unistd.h>
#include "game_state.h"

GameInfo::GameInfo(const std::string &server_name, uint8_t player_count, coordinate_t size_x,
                   coordinate_t size_y, uint16_t game_length, uint16_t explosion_radius,
                   uint16_t bomb_timer, uint64_t turn_duration, uint16_t initial_blocks,
                   const std::shared_ptr<RandomGenerator> &rng,
                   const std::shared_ptr<asio::io_context> &io_context)
        : server_name(server_name), player_count(player_count), size_x(size_x),
          size_y(size_y), game_length(game_length), explosion_radius(explosion_radius),
          bomb_timer(bomb_timer), turn_duration(turn_duration),
          initial_blocks(initial_blocks), rng(rng), io_context(io_context),
          timer(*io_context, asio::chrono::milliseconds(turn_duration)) {
    rng->set_sizes(size_x, size_y);
}

bool GameInfo::game_has_started() {
    return curr_player_id == player_count;
}

void GameInfo::serialize_hello(const send_buffer_ptr &buffer) {
    buffer->copy_into(HELLO_ID);
    buffer->copy_string_into(server_name);
    buffer->copy_into(player_count);
    buffer->copy_into(size_x);
    buffer->copy_into(size_y);
    buffer->copy_into(game_length);
    buffer->copy_into(explosion_radius);
    buffer->copy_into(bomb_timer);
}

void GameInfo::serialize_accepted_player(const send_buffer_ptr &buffer, Player::player_id_t id,
                                         const std::shared_ptr<Player> &player) {
    buffer->copy_into(ACCEPTED_PLAYER_ID);
    buffer->copy_into(id);
    player->serialize(buffer);
}

void GameInfo::new_connection(asio::ip::tcp::socket socket) {
    std::shared_ptr<Connection> connection =
            std::make_shared<Connection>(std::move(socket), this, connected_ids);
    active_users.emplace(connected_ids, connection);

    // Send Hello to new client.
    serialize_hello(connection->get_send_buff());
    connection->send_message();

    // Check if game has already started, send proper communicate.
    if (game_has_started()) {
        serialize_game_started(connection->get_send_buff());
        connection->send_message();
        for (auto &turn: past_turns) {
            size_t mess_size = turn->get_bytes_no();
            connection->get_send_buff()->check_resize(mess_size);
            memcpy(&connection->get_send_buff()->get_buffer()->at(0),
                   &turn->get_buffer()->at(0), mess_size);
            connection->get_send_buff()->set_size(mess_size);
            connection->send_message();
        }
    } else {
        for (auto &[id, player]: players) {
            serialize_accepted_player(connection->get_send_buff(), id, player);
            connection->send_message();
        }
    }

    connected_ids++;
}

void GameInfo::delete_connection(Player::player_id_t init_id) {
    auto iter = active_users.find(init_id);
    if (iter != active_users.end())
        active_users.erase(iter);
}

void GameInfo::accept_player(const std::shared_ptr<Player> &player,
                             Player::player_id_t init_id) {
    // Join was already sent by this player.
    if (ids_map.contains(init_id))
        return;

    // Full lobby.
    if (game_has_started())
        return;

    ids_map.emplace(init_id, curr_player_id);
    players.emplace(curr_player_id, player);

    // Announce AcceptedPlayer.
    for (auto &[_, connection]: active_users) {
        serialize_accepted_player(connection->get_send_buff(), curr_player_id,
                                  player);
        connection->send_message();
    }

    curr_player_id++;
    // Check if game may start.
    if (game_has_started()) {
        // Turn 0 is a special case.
        process_turn0();
        process_game_timer();
    }
}

void GameInfo::serialize_game_started(const send_buffer_ptr &buffer) {
    buffer->copy_into(GAME_STARTED_ID);
    serialize_map(buffer, players);
}

void GameInfo::serialize_turn(const send_buffer_ptr &buffer, EventVariant &events) {
    buffer->copy_into(TURN_ID);
    buffer->copy_into(turn_no);
    events.serialize(buffer);
}

void GameInfo::serialize_game_ended(const send_buffer_ptr &buffer) {
    buffer->copy_into(GAME_ENDED_ID);
    serialize_map(buffer, scores);
}

void GameInfo::reset() {
    curr_player_id = 0;
    curr_bomb_id = 0;
    turn_no = 0;
    players.clear();
    active_users.clear();
    ids_map.clear();
    player_positions.clear();
    blocks.clear();
    bombs.clear();
    scores.clear();
    past_turns.clear();
}

void GameInfo::place_bomb(Bomb::bomb_id_t id, const std::shared_ptr<Bomb> &bomb) {
    bombs.emplace(id, bomb);
}

void GameInfo::block_tile(Position position) {
    blocks.emplace_back(position);
}

bool GameInfo::is_blocked(coordinate_t x, coordinate_t y, BombExploded &bomb_exploded) {
    Position new_pos(x, y);
    for (auto &pos: blocks) {
        if (new_pos.equals(pos)) {
            bomb_exploded.insert_destroyed_block(pos);
            tiles_to_unblock.emplace_back(pos);
            return true;
        }
    }
    return false;
}

bool GameInfo::tile_blocked(coordinate_t x, coordinate_t y) {
    Position new_pos(x, y);
    for (auto &pos: blocks) {
        if (new_pos.equals(pos))
            return true;
    }
    return false;
}

void GameInfo::move_player(Player::player_id_t player_id, Position position) {
    auto iter = player_positions.find(player_id);
    if (iter != player_positions.end())
        player_positions.erase(iter);
    player_positions.emplace(player_id, position);
}

void GameInfo::unblock_tile(Position position) {
    for (size_t i = 0; i < blocks.size(); i++) {
        // If we find matching position (blocked positions are unique)
        // we swap it with last element in vector and then pop it.
        if (blocks[i].equals(position)) {
            std::swap(blocks[i], blocks.back());
            blocks.pop_back();
        }
    }
}

void GameInfo::bomb_exploded(Bomb::bomb_id_t id) {
    auto iter = bombs.find(id);
    if (iter != bombs.end())
        bombs.erase(iter);
}

void GameInfo::update_score(Player::player_id_t player_id) {
    scores.at(player_id)->increase();
}

void GameInfo::destroy_robots(coordinate_t x, coordinate_t y, BombExploded &bomb_exploded) {
    Position new_pos(x, y);
    for (auto &[player_id, position]: player_positions) {
        if (new_pos.equals(position)) {
            bomb_exploded.insert_destroyed_robot(player_id);
            dead_players.insert(player_id);
        }
    }
}

void GameInfo::decrease_bombs_timer(EventVariant &events) {
    for (auto &[bomb_id, bomb]: bombs) {
        bomb->decrease_timer();
        if (!bomb->explosion())
            continue;
        exploded_bombs.emplace_back(bomb_id);
        BombExploded bomb_exploded(bomb_id);

        // Handling explosions properly.
        auto bomb_pos = bomb->get_position();
        auto x = bomb_pos.get_x();
        auto y = bomb_pos.get_y();

        destroy_robots(x, y, bomb_exploded);
        if (is_blocked(x, y, bomb_exploded)) {
            EventVariant::variant_t new_event = bomb_exploded;
            events.insert_event(new_event);
            return;
        }
        for (coordinate_t i = 1; i <= explosion_radius && x >= i; i++) {
            destroy_robots(x - i, y, bomb_exploded);
            if (is_blocked(x - i, y, bomb_exploded))
                break;
        }
        for (coordinate_t i = 1; i <= explosion_radius && y >= i; i++) {
            destroy_robots(x, y - i, bomb_exploded);
            if (is_blocked(x, y - i, bomb_exploded))
                break;
        }
        for (coordinate_t i = 1; i <= explosion_radius && x < size_x - i; i++) {
            destroy_robots(x + i, y, bomb_exploded);
            if (is_blocked(x + i, y, bomb_exploded))
                break;
        }
        for (coordinate_t i = 1; i <= explosion_radius && y < size_y - i; i++) {
            destroy_robots(x, y + i, bomb_exploded);
            if (is_blocked(x, y + i, bomb_exploded))
                break;
        }

        EventVariant::variant_t new_event = bomb_exploded;
        events.insert_event(new_event);
    }
}

void GameInfo::make_single_move(EventVariant &events, Player::player_id_t player_id,
                                MoveVariant::move_variant_t &move) {
    auto position = player_positions.at(player_id);
    if (std::holds_alternative<PlaceBomb>(move)) {
        Bomb bomb(position, bomb_timer);
        place_bomb(curr_bomb_id, std::make_shared<Bomb>(bomb));
        BombPlaced bomb_placed(curr_bomb_id++, position);
        EventVariant::variant_t new_event = bomb_placed;
        events.insert_event(new_event);
    } else if (std::holds_alternative<PlaceBlock>(move)) {
        if (!tile_blocked(position.get_x(), position.get_y())) {
            block_tile(position);
            BlockPlaced block_placed(position);
            EventVariant::variant_t new_event = block_placed;
            events.insert_event(new_event);
        }
    } else if (std::holds_alternative<Move>(move)) {
        Move new_move = std::get<Move>(move);
        coordinate_t x = position.get_x(), y = position.get_y();

        // Check if after move player won't go outside the board.
        if (new_move.direction == Direction::UP && y != size_y - 1)
            y++;
        else if (new_move.direction == Direction::RIGHT && x != size_x - 1)
            x++;
        else if (new_move.direction == Direction::DOWN && y != 0)
            y--;
        else if (new_move.direction == Direction::LEFT && x != 0)
            x--;
        else
            return;

        // Check if after move player won't encounter tile.
        if (!tile_blocked(x, y)) {
            Position new_pos(x, y);
            move_player(player_id, new_pos);
            PlayerMoved player_moved(player_id, new_pos);
            EventVariant::variant_t new_event = player_moved;
            events.insert_event(new_event);
        }
    }
}

void GameInfo::make_moves(EventVariant &events) {
    for (auto &[player_id, _] : players) {
        auto iter = dead_players.find(player_id);
        // Checking if player died during current turn.
        if (iter != dead_players.end()) {
            Position position(rng->rand_x(), rng->rand_y());
            Debug::log(position.get_x(), " ", position.get_y());
            move_player(player_id, position);
            PlayerMoved player_moved(player_id, position);
            EventVariant::variant_t new_event = player_moved;
            events.insert_event(new_event);
        } else {
            for (auto &[p_id, move] : moves.moves) {
                if (p_id == player_id) {
                    make_single_move(events, player_id, move);
                    break;
                }
            }
        }
    }
}


void GameInfo::proceed_turn(EventVariant &events) {
    decrease_bombs_timer(events);

    for (auto &position: tiles_to_unblock)
        unblock_tile(position);
    for (auto &player_id: dead_players)
        update_score(player_id);
    for (auto &bomb_id: exploded_bombs)
        bomb_exploded(bomb_id);

    make_moves(events);

    turn_no++;
}

void GameInfo::insert_move(Player::player_id_t init_id, MoveVariant::move_variant_t &move) {
    // Moves are not allowed in Lobby.
    if (!game_has_started())
        return;
    // Observers cannot make moves.
    if (!ids_map.contains(init_id))
        return;
    moves.insert_event(ids_map.at(init_id), move);
}

void GameInfo::send_curr_turn(const send_buffer_ptr &buffer) {
    // Copy turn serialization into all buffers and past turns.
    size_t mess_size = buffer->get_bytes_no();
    std::shared_ptr<SendBuffer> this_turn = std::make_shared<SendBuffer>(mess_size);
    memcpy(&this_turn->get_buffer()->at(0), &buffer->get_buffer()->at(0), mess_size);
    past_turns.emplace_back(this_turn);
    for (auto &[_, connection]: active_users) {
        connection->get_send_buff()->check_resize(mess_size);
        memcpy(&connection->get_send_buff()->get_buffer()->at(0),
               &buffer->get_buffer()->at(0), mess_size);
        connection->get_send_buff()->set_size(mess_size);
        connection->send_message();
    }
}

void GameInfo::process_turn0() {
    // Sending GameStarted communicates.
    for (auto &[id, connection]: active_users) {
        serialize_game_started(connection->get_send_buff());
        connection->send_message();
    }

    send_buffer_ptr turn_buffer = std::make_shared<SendBuffer>();
    EventVariant events;

    // Place players and set their scores to 0.
    for (auto &[player_id, _]: players) {
        coordinate_t x = rng->rand_x();
        coordinate_t y = rng->rand_y();
        Position position(x, y);
        PlayerMoved player_moved(player_id, position);
        EventVariant::variant_t new_event = player_moved;
        events.insert_event(new_event);
        std::shared_ptr<Score> score = std::make_shared<Score>();
        move_player(player_id, position);
        scores.emplace(player_id, score);
    }

    // Place blocks without duplicates.
    for (uint16_t i = 0; i < initial_blocks; i++) {
        coordinate_t x = rng->rand_x(), y = rng->rand_y();
        if (tile_blocked(x, y))
            continue;
        Position position(x, y);
        BlockPlaced block_placed(position);
        EventVariant::variant_t new_event = block_placed;
        events.insert_event(new_event);
        block_tile(position);
    }

    serialize_turn(turn_buffer, events);
    send_curr_turn(turn_buffer);
}

void GameInfo::process_game_timer() {
    // New turn - wait for proper number of milliseconds.
    timer.expires_from_now(asio::chrono::milliseconds(turn_duration));
    timer.async_wait(
            [this](const boost::system::error_code &error_code) {
                if (!error_code)
                    process_game();
                else
                    process_game_timer();
            });
}

void GameInfo::process_game() {
    EventVariant events;
    send_buffer_ptr buffer = std::make_shared<SendBuffer>();
    proceed_turn(events);

    // Clear modified containers after the turn.
    dead_players.clear();
    tiles_to_unblock.clear();
    exploded_bombs.clear();
    moves.moves.clear();
    // Sending Turn communicates.
    serialize_turn(buffer, events);
    send_curr_turn(buffer);

    if (turn_no == game_length) {
        // Sending GameEnded communicates.
        for (auto &[_, connection]: active_users) {
            serialize_game_ended(connection->get_send_buff());
            connection->send_message();
        }

        reset();
    }
    else {
        process_game_timer();
    }
}
