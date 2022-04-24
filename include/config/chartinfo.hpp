#ifndef CHARTINFO_HPP
#define CHARTINFO_HPP

#include <list>
#include <string>

#include "config/note.hpp"
#include "config/notesequence.hpp"
#include "config/stop.hpp"
#include "config/skip.hpp"
#include "config/timeinfo.hpp"

struct ChartInfo {
    ChartInfo(std::string chartPath);
    ChartInfo(int level, std::string typist, std::string keyboardLayout);

    void loadChart(std::string chartPath);
    void saveChart(std::string chartPath);

    int level;

    float offsetMS = 0.f;

    std::string typist;
    std::string keyboardLayout;

    std::list<Timeinfo> timeinfo;

    NoteSequence notes;
};

#endif // CHARTINFO_HPP