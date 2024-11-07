// Communicates used in gui-client and client-server messages.
// Communicates describe moves performed by player.

#ifndef ZAD2_MOVE_MESSAGE_H
#define ZAD2_MOVE_MESSAGE_H

#include "utils.h"
#include "map_objects.h"

class Join {
private:
    std::string name{};

public:
    constexpr static uint8_t ID = 0;
    Join(const std::string &name);

    void serialize(const send_buffer_ptr &buffer);
};

class PlaceBomb {
public:
    constexpr static uint8_t ID = 1;
    void serialize(const send_buffer_ptr &buffer);
};

class PlaceBlock {
public:
    constexpr static uint8_t ID = 2;
    void serialize(const send_buffer_ptr &buffer);
};

class Move {
private:
    Direction direction{};

public:
    constexpr static uint8_t ID = 3;
    void serialize(const send_buffer_ptr &buffer);
    void deserialize(const recv_buffer_ptr &buffer);
};

#endif //ZAD2_MOVE_MESSAGE_H
