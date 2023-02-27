#ifndef CHARTINFO_HPP
#define CHARTINFO_HPP

#include <string>

#include <filesystem>
namespace fs = std::filesystem;

#include "config/note.hpp"
#include "config/notesequence.hpp"
#include "config/stop.hpp"
#include "config/skip.hpp"

class SongPosition;

struct ChartInfo {
    ChartInfo();
    ChartInfo(int level, std::string typist, std::string keyboardLayout, std::string difficulty);

    bool loadChart(fs::path chartPath, SongPosition & songpos);
    void saveChart(fs::path chartPath, SongPosition & songpos);

    int level;

    std::string typist;
    std::string keyboardLayout;

    std::string difficulty;

    fs::path savePath = "";

    NoteSequence notes;
};

#endif // CHARTINFO_HPP