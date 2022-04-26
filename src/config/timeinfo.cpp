#include "config/timeinfo.hpp"

Timeinfo::Timeinfo(BeatPos beatpos, Timeinfo * prevTimeinfo, int beatsPerMeasure, float bpm) :
        beatpos(beatpos), beatsPerMeasure(beatsPerMeasure), bpm(bpm), absBeatStart(calculateBeatStart()),
        absTimeStart(calculateTimeStart(prevTimeinfo)) {}

float Timeinfo::calculateBeatStart() {
    return beatpos.measure + (beatpos.split / (float)beatpos.beatsplit);
}

float Timeinfo::calculateTimeStart(Timeinfo * prevTimeinfo) {
    if(prevTimeinfo == nullptr) {
        return 0.f;
    } else {
        auto prevSectionBeatLength = absBeatStart - prevTimeinfo->absBeatStart;
        return prevTimeinfo->absTimeStart + prevSectionBeatLength * (60.f / prevTimeinfo->bpm); 
    }
}

bool operator<(const Timeinfo & lhs, const Timeinfo & rhs) {
    return lhs.beatpos < rhs.beatpos;
}