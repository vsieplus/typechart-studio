#ifndef TIMEINFO_HPP
#define TIMEINFO_HPP

#include "config/beatpos.hpp"

namespace constants {
    const int DEFAULT_BEATS_PER_MEASURE = 4;
}

struct Timeinfo {
    Timeinfo(BeatPos beatpos, Timeinfo * prevTimeinfo, int beatsPerMeasure, float bpm, float interpolateBeatDuration);

    float calculateBeatStart(Timeinfo * prevTimeinfo);
    float calculateTimeStart(Timeinfo * prevTimeinfo);

    BeatPos beatpos;
    int beatsPerMeasure;

    float bpm;

    float absBeatStart;
    float absTimeStart;

    float interpolateBeatDuration;
};


bool operator<(const Timeinfo & lhs, const Timeinfo & rhs);


#endif // TIMEINFO_HPP