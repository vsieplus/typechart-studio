#ifndef BEATPOS_HPP
#define BEATPOS_HPP

struct BeatPos {
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
