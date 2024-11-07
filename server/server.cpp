#include <iostream>
#include "input_parser.h"
#include "utils.h"

namespace {
    std::shared_ptr<GameInfo> game_info;

    void accept_connection(asio::ip::tcp::acceptor &acceptor) {
        acceptor.async_accept(
                [&acceptor](boost::system::error_code error_code,
                            asio::ip::tcp::socket socket) {
                    if (!error_code)
                        game_info->new_connection(std::move(socket));

                    accept_connection(acceptor);
                });
    }
}

int main(int argc, char *argv[]) {
    // Parsing input from command line.
    std::shared_ptr<ServerOptions> pr_opts;
    try {
        pr_opts = std::make_shared<ServerOptions>(argc, argv);
    }
    catch (const std::exception &e) {
        TerminatingError::fatal(e.what());
    }

    try {
        std::shared_ptr<asio::io_context> io_context =
                std::make_shared<asio::io_context>();
        pr_opts->initialize_game_state(game_info, io_context);
        // Create own endpoint.
        asio::ip::tcp::endpoint ep(asio::ip::tcp::v6(), pr_opts->get_port());
        // Used to accept connections from many clients.
        asio::ip::tcp::acceptor acceptor(*io_context, ep);
        Debug::log("Server is alive on ", ep.address(), ":", ep.port(), ".");

        while (true) {
            try {
                accept_connection(acceptor);
                io_context->run();
            }
            catch (const std::exception &e) {
                accept_connection(acceptor);
            }
        }
    }
    catch (const std::exception &e) {
        TerminatingError::fatal(e.what());
    }

    return 0;
}