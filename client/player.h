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

    void deserialize(const recv_buffer_ptr &buffer);

    void increase();

    score_t get_score();
};

class Player {
private:
    std::string name, address;

public:
    using player_id_t = uint8_t;
    using player_map_t = std::map<player_id_t, std::shared_ptr<Player>>;

    void serialize(const send_buffer_ptr &buffer);

    void deserialize(const recv_buffer_ptr &buffer);
};


#endif //ZAD2_PLAYER_H
