// Used to connect and communicate with TCP robot-server.

#ifndef ZAD2_SERVER_H
#define ZAD2_SERVER_H

#include <boost/array.hpp>
#include "utils.h"

class Server {
private:
    std::shared_ptr<asio::io_context> io_context;
    asio::ip::tcp::socket socket;

public:
    Server(const std::shared_ptr<asio::io_context> &io_context,
           const std::shared_ptr<Address> &address);

    virtual ~Server();

    // Receive full message from server.
    void receive_message(const recv_buffer_ptr &buffer);

    // Send message to server.
    void send_message(const send_buffer_ptr &buffer);
};


#endif //ZAD2_SERVER_H
