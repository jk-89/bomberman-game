#include <boost/lexical_cast.hpp>
#include "utils.h"
#include "client_connection.h"
#include "move.h"
#include "game_state.h"

std::string Connection::get_client_addres() {
    return boost::lexical_cast<std::string>(socket.remote_endpoint());
}

Connection::Connection(asio::ip::tcp::socket socket_, GameInfo *game_info,
                       Player::player_id_t id) : socket(std::move(socket_)),
                                                 game_info(game_info), id(id) {
    // Turn off Nagle's algorithm.
    socket.set_option(asio::ip::tcp::no_delay(true));
    send_buff = std::make_shared<SendBuffer>();
    recv_buff = std::make_shared<ReceiveBuffer>();
    Debug::log("Client ", get_client_addres(), " with id = ", (uint16_t) id, " connected.");
    // Start listening.
    receive_message();
}

void Connection::disconnect() {
    game_info->delete_connection(id);
    Debug::log("Disconnecting client ", get_client_addres(), ".");
}

void Connection::try_to_parse() {
    recv_buff->reset_processed();
    // Check which message type was received from client.
    uint8_t mess_id;
    recv_buff->copy_from(mess_id);
    if (mess_id == Join::ID) {
        std::shared_ptr<Join> join = std::make_shared<Join>();
        join->deserialize(recv_buff);
        Debug::log("Client ", (uint16_t) id, " sent Join: ", join->get_name(), ".");
        std::shared_ptr<Player> player = std::make_shared<Player>
                (join->get_name(), get_client_addres());
        game_info->accept_player(player, id);
    } else if (mess_id == PlaceBomb::ID) {
        Debug::log("Client ", (uint16_t) id, " sent PlaceBomb.");
        PlaceBomb place_bomb;
        MoveVariant::move_variant_t new_move = place_bomb;
        game_info->insert_move(id, new_move);
    } else if (mess_id == PlaceBlock::ID) {
        Debug::log("Client ", (uint16_t) id, " sent PlaceBlock.");
        PlaceBlock place_block;
        MoveVariant::move_variant_t new_move = place_block;
        game_info->insert_move(id, new_move);
    } else if (mess_id == Move::ID) {
        Move move;
        move.deserialize(recv_buff);
        Debug::log("Client ", (uint16_t) id, " sent Move: ", (uint16_t) move.get_direction(), ".");
        MoveVariant::move_variant_t new_move = move;
        game_info->insert_move(id, new_move);
    } else {
        throw WrongMessage();
    }

    // If message was properly parsed then erase it from buffer.
    recv_buff->reload();
}

void Connection::receive_message() {
    recv_buff->enlarge_if_full();
    socket.async_receive(
            asio::buffer(*recv_buff->get_buffer()) + recv_buff->get_bytes_no(),
            [this](boost::system::error_code error_code, size_t recvd) {
                if (!error_code) {
                    recv_buff->increase_size(recvd);
                    // Nothing to parse.
                    if (recv_buff->get_bytes_no() == 0)
                        receive_message();

                    try {
                        try_to_parse();
                    }
                    catch (const MessageTooShort &e) {
                        receive_message();
                    }
                    catch (const WrongMessage &e) {
                        Debug::log("Client ", (uint16_t) id, " sent improper message.");
                        disconnect();
                    }
                } else {
                    Debug::log("Error occurred while receiving message from client ",
                               (uint16_t) id, ".");
                    disconnect();
                }

                receive_message();
            });
}

void Connection::send_message() {
    std::cerr << "-- ";
    for (int i = 0; i < (int) send_buff->get_bytes_no(); i++)
        std::cerr << (uint16_t) send_buff->get_buffer()->at(i) << " ";
    std::cerr << '\n';

    async_write(socket,
            asio::buffer(*send_buff->get_buffer(), send_buff->get_bytes_no()),
            [this](boost::system::error_code error_code, size_t sent) {
                if (error_code)
                    disconnect();
                Debug::log("Sent ", sent, " bytes to client ", (uint16_t) id, ".");
            });
}

send_buffer_ptr &Connection::get_send_buff() {
    send_buff->reset_state();
    return send_buff;
}
