#ifndef SONGPOSITION_HPP
#define SONGPOSITION_HPP

struct SongPosition {
    float absTime = 0.f;
    float absBeat = 0.f;

    unsigned long songStart = 0;

    int currentSection = 0;

    std::vector<Timeinfo> timeinfo;
};

#endif // SONGPOSITION_HPP