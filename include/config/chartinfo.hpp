#ifndef CHARTINFO_HPP
#define CHARTINFO_HPP

#include <string>

#include "config/note.hpp"
#include "config/notesequence.hpp"
#include "config/stop.hpp"
#include "config/skip.hpp"

struct ChartInfo {
    ChartInfo(std::string chartPath);
    ChartInfo(int level, std::string typist, std::string keyboardLayout);

    void loadChart(std::string chartPath);
    void saveChart(std::string chartPath);

    int level;

    std::string typist;
    std::string keyboardLayout;

    std::string savePath = "";

    NoteSequence notes;
};

#endif // CHARTINFO_HPP