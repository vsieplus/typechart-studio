#ifndef CHARTINFO_HPP
#define CHARTINFO_HPP

#include <filesystem>
#include <string>
#include <string_view>

#include <json.hpp>

#include "config/note.hpp"
#include "config/notesequence.hpp"
#include "config/notesequenceitem.hpp"
#include "config/stop.hpp"
#include "config/skip.hpp"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
namespace fs = std::filesystem;

class SongPosition;

struct ChartInfo {
    ChartInfo() = default;
    ChartInfo(int level, std::string_view typist, std::string_view keyboardLayout, std::string_view difficulty);

    // load chart data from the given path
    bool loadChart(const fs::path & chartPath, SongPosition & songpos);

    // load chart metadata from the given json
    void loadChartMetadata(ordered_json chartinfoJSON);

    // load chart data
    void loadChartTimeInfo(ordered_json chartinfoJSON, SongPosition & songpos) const;
    void loadChartStops(ordered_json chartinfoJSON, SongPosition & songpos);
    void loadChartSkips(ordered_json chartinfoJSON, SongPosition & songpos);
    void loadChartNotes(ordered_json chartinfoJSON, SongPosition & songpos);

    BeatPos findMatchingReleaseNote(std::string_view keyText, std::vector<ordered_json>::iterator iter, std::vector<ordered_json> notesJSON) const;
    NoteSequenceItem::SequencerItemType determineItemType(const std::string & keyText) const;

    // save chart data to the given path
    void saveChart(const fs::path & chartPath, SongPosition & songpos);

    // save chart metadata
    ordered_json saveChartMetadata(const SongPosition & songpos) const;

    // save chart data
    ordered_json saveChartTimeInfo(SongPosition & songpos) const;

    int level { 0 };

    std::string typist {};
    std::string keyboardLayout {};
    std::string difficulty {};

    fs::path savePath {};

    NoteSequence notes;
};

#endif // CHARTINFO_HPP
