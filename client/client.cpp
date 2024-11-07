// Main client file. Client works on two threads, one is responsible
// for receiving messages from server and sending them to GUI
// and second vice-versa.

#include <exception>
#include <thread>
#include "utils.h"
#include "input_parser.h"
#include "server.h"
#include "gui.h"
#include "server_message.h"
#include "game_state.h"
#include "move_message.h"

namespace {
    std::shared_ptr<ProgramOptions> pr_opts;

    // Parsing message until it is fully loaded.
    template<typename T>
    void wait_full_message(Server &server, const recv_buffer_ptr &buffer,
                           std::shared_ptr<T> &mess) {
        while (true) {
            mess = std::make_shared<T>();
            buffer->reset_processed();
            uint8_t mess_id;
            buffer->copy_from(mess_id);
            try {
                mess->deserialize(buffer);
            }
            catch (const MessageTooShort &e) {
                server.receive_message(buffer);
                continue;
            }
            break;
        }
        Debug::log("Full message size ", buffer->get_bytes_no(), " loaded from server.");
        buffer->reload();
    }

    void server_to_gui(Server &server, Gui &gui, GameInfo &game_info) {
        std::shared_ptr<SendBuffer> send_buff = std::make_shared<SendBuffer>();
        std::shared_ptr<ReceiveBuffer> recv_buff = std::make_shared<ReceiveBuffer>();

        try {
            while (true) {
                send_buff->reset_state();
                if (recv_buff->get_bytes_no() == 0)
                    server.receive_message(recv_buff);

                // Check which message type was received from server.
                uint8_t mess_id = recv_buff->get_buffer()->at(0);
                if (mess_id == Hello::ID) {
                    Debug::log("Server sent Hello.");
                    std::shared_ptr<Hello> hello;
                    wait_full_message(server, recv_buff, hello);
                    hello->change_info(game_info);
                } else if (mess_id == AcceptedPlayer::ID) {
                    Debug::log("Server sent AcceptedPlayer.");
                    std::shared_ptr<AcceptedPlayer> accepted_player;
                    wait_full_message(server, recv_buff, accepted_player);
                    accepted_player->change_info(game_info);
                } else if (mess_id == GameStarted::ID) {
                    Debug::log("Server sent GameStarted.");
                    std::shared_ptr<GameStarted> game_started;
                    wait_full_message(server, recv_buff, game_started);
                    game_started->change_info(game_info);
                } else if (mess_id == Turn::ID) {
                    Debug::log("Server sent Turn.");
                    std::shared_ptr<Turn> turn;
                    wait_full_message(server, recv_buff, turn);
                    turn->change_info(game_info);
                } else if (mess_id == GameEnded::ID) {
                    Debug::log("Server sent GameEnded.");
                    std::shared_ptr<GameEnded> game_ended;
                    wait_full_message(server, recv_buff, game_ended);
                    game_info.reset();
                } else {
                    throw WrongMessage();
                }

                if (mess_id != GameStarted::ID) {
                    game_info.serialize(send_buff);
                    gui.send_message(send_buff);
                }
            }
        }
        catch (const std::exception &e) {
            TerminatingError::fatal(e.what());
        }
    }

    void gui_to_server(Server &server, Gui &gui, GameInfo &game_info) {
        std::shared_ptr<SendBuffer> send_buff = std::make_shared<SendBuffer>();
        std::shared_ptr<ReceiveBuffer> recv_buff = std::make_shared<ReceiveBuffer>();

        try {
            while (true) {
                recv_buff->reset_state();
                send_buff->reset_state();

                gui.receive_message(recv_buff);
                if (recv_buff->get_bytes_no() == 0)
                    continue;
                uint8_t mess_id;
                recv_buff->copy_from(mess_id);
                // Check type and wrong messages from GUI.
                try {
                    if (mess_id == PlaceBomb::ID - 1 || mess_id == PlaceBlock::ID - 1) {

                    } else if (mess_id == Move::ID - 1) {
                      Move move;
                      move.deserialize(recv_buff);
                    } else {
                        throw WrongMessage();
                    }
                    if (!recv_buff->did_read_all())
                        throw WrongMessage();
                }
                catch (const WrongMessage &e) {
                    continue;
                }
                catch (const MessageTooShort &e) {
                    continue;
                }

                // Convert message to join if game is in Lobby.
                if (game_info.get_state() == GAME) {
                    recv_buff->get_buffer()->at(0)++;
                    auto size = recv_buff->get_bytes_no();
                    for (size_t i = 0; i < size; i++)
                        send_buff->copy_into(recv_buff->get_buffer()->at(i));
                } else {
                    Join join(pr_opts->get_name());
                    join.serialize(send_buff);
                }
                server.send_message(send_buff);
            }
        }
        catch (const std::exception &e) {
            TerminatingError::fatal(e.what());
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        // Parsing input from command line.
        pr_opts = std::make_shared<ProgramOptions>(argc, argv);

        // Create sockets and threads.
        std::shared_ptr<asio::io_context> gui_context = std::make_shared<asio::io_context>();
        std::shared_ptr<asio::io_context> srv_context = std::make_shared<asio::io_context>();
        Gui gui(gui_context, pr_opts->get_gui_address(), pr_opts->get_port());
        Server server(srv_context, pr_opts->get_server_address());
        GameInfo game_info;

        std::thread t1(server_to_gui, std::ref(server),
                       std::ref(gui), std::ref(game_info));
        std::thread t2(gui_to_server, std::ref(server),
                       std::ref(gui), std::ref(game_info));
        t1.join();
        t2.join();
    }
    catch (const std::exception &e) {
        TerminatingError::fatal(e.what());
    }

    return 0;
}
