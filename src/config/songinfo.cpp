#include "config/songinfo.hpp"

#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void SongInfo::saveSonginfo(std::string savePath) {
    // save simple json file with songinfo metadata
    json songinfo;

    songinfo["title"] = title;
    songinfo["artist"] = artist;
    songinfo["music"] = musicFilename;
    songinfo["coverart"] = coverartFilename;
    songinfo["bpmtext"] = bpmtext;
    songinfo["musicPreviewStart"] = musicPreviewStart;
    songinfo["musicPreviewStop"] = musicPreviewStop;

    std::ofstream file(savePath.c_str());
    file << std::setw(4) << songinfo << std::endl;
}