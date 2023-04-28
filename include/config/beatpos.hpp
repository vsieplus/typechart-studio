#ifndef BEATPOS_HPP
#define BEATPOS_HPP

#include <vector>

namespace constants {
    const int NUM_BEATPOS_ELEMENTS = 3;
}

struct BeatPos {
    BeatPos();
    BeatPos(int measure, int measureSplit, int split);

    int measure;
    int measureSplit;
    int split;
};

BeatPos operator+(const BeatPos & lhs, const BeatPos & rhs);
BeatPos operator-(const BeatPos & lhs, const BeatPos & rhs);

bool operator==(const BeatPos & lhs, const BeatPos & rhs);
bool operator!=(const BeatPos & lhs, const BeatPos & rhs);
bool operator<(const BeatPos & lhs, const BeatPos & rhs);

#endif // BEATPOS_HPP
