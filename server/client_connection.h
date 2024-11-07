// Used to asynchronously connect and communicate with single TCP robot-client.

#ifndef ZAD2_CLIENT_CONNECTION_H
#define ZAD2_CLIENT_CONNECTION_H

class GameInfo;
#include <boost/array.hpp>
#include "utils.h"
#include "player.h"

class Connection {
private:
    asio::ip::tcp::socket socket;
    send_buffer_ptr send_buff;
    recv_buffer_ptr recv_buff;
    GameInfo *game_info;
    Player::player_id_t id;

    // Try to parse a message from the client.
    void try_to_parse();

    // Disconnect client from server.
    void disconnect();

    // Retrieves client address from socket.
    std::string get_client_addres();

public:
    Connection(asio::ip::tcp::socket socket, GameInfo *game_info,
               Player::player_id_t id);

    // Receive message from client and properly modify game state.
    void receive_message();

    send_buffer_ptr &get_send_buff();

    // Send message to client.
    void send_message();
};


#endif //ZAD2_CLIENT_CONNECTION_H
