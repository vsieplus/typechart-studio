#include "config/songinfo.hpp"
#include "ui/preferences.hpp"

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace fs = std::filesystem;

void SongInfo::saveSonginfo(std::string saveDir) {
    this->saveDir = saveDir;
    fs::path songinfoSavePath = fs::path(saveDir) / fs::path("songinfo.json");

    // save simple json file with songinfo metadata
    json songinfo;

    songinfo["title"] = title;
    songinfo["artist"] = artist;
    songinfo["music"] = musicFilename;
    songinfo["coverart"] = coverartFilename;
    songinfo["bpmtext"] = bpmtext;
    songinfo["musicPreviewStart"] = musicPreviewStart;
    songinfo["musicPreviewStop"] = musicPreviewStop;

    std::ofstream file(songinfoSavePath.c_str());
    file << std::setw(4) << songinfo << std::endl;

    if(Preferences::Instance().getCopyArtAndMusic()) {
        fs::path artSavePath = fs::path(saveDir) / fs::path(coverartFilename);
        fs::path musicSavePath = fs::path(saveDir) / fs::path(musicFilename);

        fs::copy_file(fs::path(musicFilepath), musicSavePath, fs::copy_options::overwrite_existing);
        fs::copy_file(fs::path(coverartFilepath), artSavePath, fs::copy_options::overwrite_existing);
    }
}