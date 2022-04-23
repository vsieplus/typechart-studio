#ifndef STOP_HPP
#define STOP_HPP

#include "config/beatpos.hpp"

struct Stop {
    Stop(BeatPos pos, float beatDuration) : pos(pos), beatDuration(beatDuration) {}

    BeatPos pos;
    float beatDuration;
};

#endif // STOP_HPP