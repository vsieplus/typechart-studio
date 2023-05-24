#ifndef CHARTINFO_HPP
#define CHARTINFO_HPP

#include <string>

#include <filesystem>
namespace fs = std::filesystem;

#include <json.hpp>
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

#include "config/note.hpp"
#include "config/notesequence.hpp"
#include "config/stop.hpp"
#include "config/skip.hpp"

class SongPosition;

struct ChartInfo {
    ChartInfo();
    ChartInfo(int level, std::string typist, std::string keyboardLayout, std::string difficulty);

    // load chart data from the given path
    bool loadChart(fs::path chartPath, SongPosition & songpos);

    // load chart metadata from the given json
    void loadChartMetadata(ordered_json chartinfoJSON);

    // load chart data
    void loadChartTimeInfo(ordered_json chartinfoJSON, SongPosition & songpos);
    void loadChartStops(ordered_json chartinfoJSON, SongPosition & songpos);
    void loadChartSkips(ordered_json chartinfoJSON, SongPosition & songpos);
    void loadChartNotes(ordered_json chartinfoJSON, SongPosition & songpos);

    // save chart data to the given path
    void saveChart(fs::path chartPath, SongPosition & songpos);

    // save chart metadata
    ordered_json saveChartMetadata(SongPosition & songpos);

    // save chart data
    ordered_json saveChartTimeInfo(SongPosition & songpos);

    int level;

    std::string typist;
    std::string keyboardLayout;

    std::string difficulty;

    fs::path savePath = "";

    NoteSequence notes;
};

#endif // CHARTINFO_HPP
