#include "config/songinfo.hpp"
#include "ui/preferences.hpp"

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace fs = std::filesystem;

void SongInfo::saveSonginfo(std::string saveDir, bool initialSaved) {
    this->saveDir = saveDir;
    fs::path songinfoSavePath = fs::path(saveDir) / fs::path("songinfo.json");

    // save simple json file with songinfo metadata
    ordered_json songinfo;

    songinfo["title"] = title;
    songinfo["artist"] = artist;
    songinfo["genre"] = genre;
    songinfo["music"] = musicFilename;
    songinfo["coverart"] = coverartFilename;
    songinfo["bpmtext"] = bpmtext;
    songinfo["musicPreviewStart"] = musicPreviewStart;
    songinfo["musicPreviewStop"] = musicPreviewStop;

    std::ofstream file(songinfoSavePath.c_str());
    file << std::setw(4) << songinfo << std::endl;

    if(Preferences::Instance().getCopyArtAndMusic() && !initialSaved) {
        fs::path artSavePath = fs::path(saveDir) / fs::path(coverartFilename);
        fs::path musicSavePath = fs::path(saveDir) / fs::path(musicFilename);

        fs::path musicSrcPath = fs::path(musicFilepath);
        if(fs::exists(musicSrcPath) && musicSrcPath != musicSavePath) {
            fs::copy_file(musicSrcPath, musicSavePath, fs::copy_options::overwrite_existing);
        }

        fs::path artSrcPath = fs::path(coverartFilepath);
        if(fs::exists(artSrcPath) && artSrcPath != artSavePath) {
            fs::copy_file(artSrcPath, artSavePath, fs::copy_options::overwrite_existing);
        }
    }
}
