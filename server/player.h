// Description of player and his score.

#ifndef ZAD2_PLAYER_H
#define ZAD2_PLAYER_H

#include "utils.h"

class Score {
public:
    using score_t = uint32_t;

private:
    score_t score = 0;

public:
    void serialize(const send_buffer_ptr &buffer);

    void increase();
};

class Player {
private:
    std::string name, address;

public:
    using player_id_t = uint8_t;

    Player(const std::string &name, const std::string &address);

    void serialize(const send_buffer_ptr &buffer);
};


#endif //ZAD2_PLAYER_H
