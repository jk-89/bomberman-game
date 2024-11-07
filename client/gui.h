// Used to connect and communicate with UDP gui-server.

#ifndef ZAD2_GUI_H
#define ZAD2_GUI_H

#include "utils.h"
#include "buffer.h"

class Gui {
private:
    std::shared_ptr<asio::io_context> io_context;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint gui_ep;

public:
    Gui(const std::shared_ptr<asio::io_context> &io_context,
        const std::shared_ptr<Address> &address, port_t my_port);

    virtual ~Gui();

    // Receive full message from server.
    void receive_message(recv_buffer_ptr &buffer);

    // Send message to server.
    void send_message(const send_buffer_ptr &buffer);
};


#endif //ZAD2_GUI_H
