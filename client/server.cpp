#include "server.h"

Server::Server(const std::shared_ptr<asio::io_context> &io_context_,
               const std::shared_ptr<Address> &address)
        : io_context(io_context_),
          socket(*io_context_) {
    try {
        asio::ip::tcp::resolver resolver(*io_context);
        asio::ip::tcp::resolver::results_type endpoints =
                resolver.resolve(address->get_ip(), address->get_port());
        asio::connect(socket, endpoints);
        // Turn of Nagle's algorithm.
        socket.set_option(asio::ip::tcp::no_delay(true));

        Debug::log("Connected to server ", socket.remote_endpoint().address(),
                   ":", socket.remote_endpoint().port());
    }
    catch (const std::exception &e) {
        TerminatingError::fatal(e.what());
    }
};

Server::~Server() {
    Debug::log("Disconnecting from server ", socket.remote_endpoint().address(),
               ":", socket.remote_endpoint().port());
    socket.close();
}

void Server::receive_message(const recv_buffer_ptr &buffer) {
    buffer->enlarge_if_full();
    size_t recvd = socket.read_some(asio::buffer(
            *buffer->get_buffer()) + buffer->get_bytes_no());
    if (recvd == 0)
        TerminatingError::fatal("Connection failed.");
    Debug::log("Received ", recvd, " bytes from server.");
    buffer->increase_size(recvd);
}

void Server::send_message(const send_buffer_ptr &buffer) {
    socket.send(asio::buffer(*buffer->get_buffer(), buffer->get_bytes_no()));
    Debug::log("Sent ", buffer->get_bytes_no(), " bytes to server.");
}
