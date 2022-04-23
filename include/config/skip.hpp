#ifndef SKIP_HPP
#define SKIP_HPP

#include "config/beatpos.hpp"

struct Skip {
    Skip(BeatPos pos, float skipTime, float beatDuration) : pos(pos), skipTime(skipTime), beatDuration(beatDuration) {}

    BeatPos pos;

    float skipTime;
    float beatDuration;
};

#endif // STOP_HPP