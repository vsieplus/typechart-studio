#ifndef CHARTINFO_HPP
#define CHARTINFO_HPP

#include <string>

#include "config/note.hpp"
#include "config/notesequence.hpp"
#include "config/stop.hpp"
#include "config/skip.hpp"

class SongPosition;

struct ChartInfo {
    ChartInfo();
    ChartInfo(int level, std::string typist, std::string keyboardLayout);

    bool loadChart(std::string chartPath, SongPosition & songpos);
    void saveChart(std::string chartPath, SongPosition & songpos);

    int level;

    std::string typist;
    std::string keyboardLayout;

    std::string savePath = "";

    NoteSequence notes;
};

#endif // CHARTINFO_HPP