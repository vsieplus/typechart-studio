#include "config/beatpos.hpp"
#include "config/timeinfo.hpp"

#include <numeric>

BeatPos::BeatPos(int measure, int measureSplit, int split) : measure(measure), measureSplit(measureSplit), split(split) {}

BeatPos operator+(const BeatPos & lhs, const BeatPos & rhs) {
    // make sure lhs / rhs measuresplit have the same denominator
    int measureSplit = lhs.measureSplit;
    int lhsSplit = lhs.split;
    int rhsSplit = rhs.split;

    if(lhs.measureSplit != rhs.measureSplit) {
        // find lcd of lhs / rhs measuresplit
        measureSplit = std::lcm(lhs.measureSplit, rhs.measureSplit);

        // scale split accordingly
        lhsSplit = lhs.split * (measureSplit / lhs.measureSplit);
        rhsSplit = rhs.split * (measureSplit / rhs.measureSplit);
    }

    // treat beatpos as a mixed number, compute accordingly
    // measure + split / measuresplit
    auto measure = lhs.measure + rhs.measure;
    auto split = lhsSplit + rhsSplit;

    if(split >= measureSplit) {
        measure += split / measureSplit;
        split %= measureSplit;
    }

    return {measure, measureSplit, split};
}

BeatPos operator-(const BeatPos & lhs, const BeatPos & rhs) {
    // make sure lhs / rhs measuresplit have the same denominator
    int measureSplit = lhs.measureSplit;
    int lhsSplit = lhs.split;
    int rhsSplit = rhs.split;

    if(lhs.measureSplit != rhs.measureSplit) {
        // find lcd of lhs / rhs measuresplit
        measureSplit = std::lcm(lhs.measureSplit, rhs.measureSplit);

        // scale split accordingly
        lhsSplit = lhs.split * (measureSplit / lhs.measureSplit);
        rhsSplit = rhs.split * (measureSplit / rhs.measureSplit);
    }

    // treat beatpos as a mixed number, compute accordingly
    // measure + split / measuresplit
    auto measure = lhs.measure - rhs.measure;
    auto split = lhsSplit - rhsSplit;

    if(split < 0) {
        measure -= 1;
        split += measureSplit;
    }

    return {measure, measureSplit, split};
}

bool operator==(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure == rhs.measure) && (lhs.measureSplit == rhs.measureSplit) && (lhs.split == rhs.split);
}

bool operator!=(const BeatPos & lhs, const BeatPos & rhs) {
    return !(lhs == rhs);
}

bool operator<(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure < rhs.measure) ||
           ((lhs.measure == rhs.measure) && ((double)lhs.split / lhs.measureSplit < (double)rhs.split / rhs.measureSplit));
}
