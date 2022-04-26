#include "config/beatpos.hpp"

bool operator==(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure == rhs.measure) && (lhs.beatsplit == rhs.beatsplit) && (lhs.split == rhs.split);
}

bool operator<(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure < rhs.measure) ||
           ((lhs.measure == rhs.measure) && (lhs.split / lhs.beatsplit < rhs.split / rhs.beatsplit));
}