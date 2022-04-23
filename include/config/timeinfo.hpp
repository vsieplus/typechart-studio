#ifndef TIMEINFO_HPP
#define TIMEINFO_HPP

#include "config/beatpos.hpp"

struct Timeinfo {
    Timeinfo(BeatPos pos, int beatsPerMeasure, float bpm) :
        pos(pos), beatsPerMeasure(beatsPerMeasure), bpm(bpm) {}

    BeatPos pos;
    int beatsPerMeasure;

    float bpm;
};


#endif // TIMEINFO_HPP