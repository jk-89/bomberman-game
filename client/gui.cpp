#include "gui.h"

Gui::Gui(const std::shared_ptr<asio::io_context> &io_context_,
         const std::shared_ptr<Address> &address, port_t my_port)
        : io_context(io_context_),
          socket(*io_context_, asio::ip::udp::endpoint
                  (asio::ip::udp::v6(), my_port)) {
    try {
        asio::ip::udp::resolver resolver(*io_context);
        gui_ep = *resolver.resolve(address->get_ip(), address->get_port());
        Debug::log("Connected to GUI ", gui_ep.address(), ":", gui_ep.port());
    }
    catch (const std::exception &e) {
        TerminatingError::fatal(e.what());
    }
}

Gui::~Gui() {
    Debug::log("Disconnecting from GUI ", gui_ep.address(), ":", gui_ep.port());
    socket.close();
}

void Gui::receive_message(recv_buffer_ptr &buffer) {
    size_t recvd = socket.receive(asio::buffer(*buffer->get_buffer()), MSG_WAITALL);
    Debug::log("Received ", recvd, " bytes from GUI.");
    buffer->increase_size(recvd);
}

void Gui::send_message(const send_buffer_ptr &buffer) {
    socket.send_to(asio::buffer(*buffer->get_buffer(), buffer->get_bytes_no()), gui_ep);
    Debug::log("Sent ", buffer->get_bytes_no(), " bytes to GUI.");
}
