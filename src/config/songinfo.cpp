#include "config/constants.hpp"
#include "config/songinfo.hpp"
#include "ui/preferences.hpp"

#include <fstream>

#include <json.hpp>
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

SongInfo::SongInfo(std::string_view title, std::string_view artist, std::string_view genre, std::string_view bpmtext, std::string_view musicFilename,
    std::string_view coverartFilename, const fs::path & musicFilepath, const fs::path & coverartFilepath, float musicPreviewStart, float musicPreviewStop)
    : title(title)
    , artist(artist)
    , genre(genre)
    , bpmtext(bpmtext)
    , musicFilename(musicFilename)
    , coverartFilename(coverartFilename)
    , musicFilepath(musicFilepath)
    , coverartFilepath(coverartFilepath)
    , musicPreviewStart(musicPreviewStart)
    , musicPreviewStop(musicPreviewStop) {}

bool SongInfo::loadSongInfo(const fs::path & songinfoPath, const fs::path & songinfoDir) {
    saveDir = songinfoDir;

    json songinfoJSON;

    try {
        std::ifstream in(songinfoPath);
        in >> songinfoJSON;
    } catch(json::exception & e) {
        return false;
    }

    title = songinfoJSON.value(constants::TITLE_KEY, "");
    artist = songinfoJSON.value(constants::ARTIST_KEY, "");
    genre = songinfoJSON.value(constants::GENRE_KEY, "");
    bpmtext = songinfoJSON.value(constants::BPMTEXT_KEY, "");

    musicFilename = songinfoJSON.value(constants::MUSIC_KEY, "");
    musicFilepath = songinfoDir / fs::path(musicFilename);
    coverartFilename = songinfoJSON.value(constants::COVERART_KEY, "");
    coverartFilepath = songinfoDir / fs::path(coverartFilename);

    musicPreviewStart = songinfoJSON.value(constants::MUSIC_PREVIEW_START_KEY, 0.f);
    musicPreviewStop = songinfoJSON.value(constants::MUSIC_PREVIEW_STOP_KEY, 0.f);
    offsetMS = songinfoJSON.value(constants::OFFSET_KEY, constants::OFFSET_VALUE_DEFAULT);

    return true;
}

void SongInfo::saveSongInfo(const fs::path & saveDir, bool initialSaved) {
    this->saveDir = saveDir;
    fs::path songinfoSavePath = saveDir / fs::path(constants::SONGINFO_FILENAME);

    // save simple json file with songinfo metadata
    ordered_json songinfo;

    songinfo[constants::TITLE_KEY] = title;
    songinfo[constants::ARTIST_KEY] = artist;
    songinfo[constants::GENRE_KEY] = genre;
    songinfo[constants::MUSIC_KEY] = musicFilename;
    songinfo[constants::COVERART_KEY] = coverartFilename;
    songinfo[constants::BPMTEXT_KEY] = bpmtext;
    songinfo[constants::MUSIC_PREVIEW_START_KEY] = musicPreviewStart;
    songinfo[constants::MUSIC_PREVIEW_STOP_KEY] = musicPreviewStop;
    songinfo[constants::OFFSET_KEY] = offsetMS;

    std::ofstream file(songinfoSavePath.c_str());
    file << std::setw(4) << songinfo << std::endl;

    if(Preferences::Instance().getCopyArtAndMusic() && !initialSaved) {
        fs::path artSavePath { saveDir / fs::path(coverartFilename) };
        fs::path musicSavePath { saveDir / fs::path(musicFilename) };

        if(fs::path musicSrcPath = musicFilepath; fs::exists(musicSrcPath) && musicSrcPath != musicSavePath) {
            fs::copy_file(musicSrcPath, musicSavePath, fs::copy_options::overwrite_existing);
        }

        if(fs::path artSrcPath = coverartFilepath; fs::exists(artSrcPath) && artSrcPath != artSavePath) {
            fs::copy_file(artSrcPath, artSavePath, fs::copy_options::overwrite_existing);
        }
    }
}

std::string SongInfo::getSongID() const {
    return title + " - " + artist;
}
