#ifndef STOP_HPP
#define STOP_HPP

#include "config/beatpos.hpp"

struct Stop {
    Stop(float absBeat, float beatDuration) : absBeat(absBeat), beatDuration(beatDuration), beatEnd(absBeat + beatDuration) {}

    float absBeat;
    float beatDuration;

    float beatEnd;
};

#endif // STOP_HPP