#include "config/beatpos.hpp"

bool operator==(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure == rhs.measure) && (lhs.measureSplit == rhs.measureSplit) && (lhs.split == rhs.split);
}

bool operator!=(const BeatPos & lhs, const BeatPos & rhs) {
    return !(lhs == rhs);
}

bool operator<(const BeatPos & lhs, const BeatPos & rhs) {
    return (lhs.measure < rhs.measure) ||
           ((lhs.measure == rhs.measure) && ((float)lhs.split / lhs.measureSplit < (float)rhs.split / rhs.measureSplit));
}
