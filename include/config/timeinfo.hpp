#ifndef TIMEINFO_HPP
#define TIMEINFO_HPP

#include "config/beatpos.hpp"

namespace constants {
    const int DEFAULT_BEATS_PER_MEASURE = 4;
}

struct Timeinfo {
    Timeinfo(BeatPos beatpos, const Timeinfo * prevTimeinfo, int beatsPerMeasure, double bpm, double interpolateBeatDuration);

    double calculateBeatStart(const Timeinfo * prevTimeinfo) const;
    double calculateTimeStart(const Timeinfo * prevTimeinfo) const;

    BeatPos beatpos;
    int beatsPerMeasure;

    double bpm;

    double absBeatStart;
    double absTimeStart;

    double interpolateBeatDuration;
};

bool operator<(const Timeinfo & lhs, const Timeinfo & rhs);

#endif // TIMEINFO_HPP
