// Map and all objects present on map.

#ifndef ZAD2_MAP_OBJECTS_H
#define ZAD2_MAP_OBJECTS_H

#include <iostream>
#include <map>
#include "utils.h"

using coordinate_t = uint16_t;
using direction_t = uint8_t;

// All four possible directions.
enum Direction {
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3
};

// Used to describe position of object on the map.
class Position {
private:
    coordinate_t x{}, y{};

public:
    Position();

    Position(coordinate_t x, coordinate_t y);

    bool equals(Position &other);

    void serialize(const send_buffer_ptr &buffer);

    void deserialize(const recv_buffer_ptr &buffer);

    coordinate_t get_x();

    coordinate_t get_y();
};

class Bomb {
public:
    using bomb_timer_t = uint16_t;
    using bomb_id_t = uint32_t;

private:
    Position position;
    bomb_timer_t timer;

public:
    Bomb(const Position &position, bomb_timer_t timer);

    void serialize(const send_buffer_ptr &buffer);

    void decrease_timer();

    Position &get_position();
};

#endif //ZAD2_MAP_OBJECTS_H
