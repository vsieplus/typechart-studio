#include "config/beatpos.hpp"

bool operator==(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure == rhs.measure) && (lhs.beatsplit == rhs.beatsplit) && (lhs.split == rhs.split);
}

bool operator!=(const BeatPos & lhs, const BeatPos & rhs) {
    return !(lhs == rhs);
}

bool operator<(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure < rhs.measure) ||
           ((lhs.measure == rhs.measure) && ((float)lhs.split / lhs.beatsplit < (float)rhs.split / rhs.beatsplit));
}
