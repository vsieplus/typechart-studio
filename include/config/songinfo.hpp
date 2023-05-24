#ifndef SONGINFO_HPP
#define SONGINFO_HPP

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

struct SongInfo {
    SongInfo();
    SongInfo(std::string title, std::string artist, std::string genre, std::string bpmtext, std::string musicFilename, std::string coverartFilename,
        fs::path musicFilepath, fs::path coverartFilepath, float musicPreviewStart, float musicPreviewStop);

    bool loadSongInfo(fs::path songinfoPath, fs::path songinfoDir);
    void saveSongInfo(fs::path saveDir, bool initialSaved);

    std::string getSongID() const;

    std::string title;
    std::string artist;
    std::string genre;

    std::string bpmtext;

    std::string musicFilename;
    std::string coverartFilename;

    // if user wishes to copy files when saving config
    fs::path musicFilepath;
    fs::path coverartFilepath;

    fs::path saveDir;

    int offsetMS = 0.f;

    float musicPreviewStart = 0.f;
    float musicPreviewStop = 0.f;
};

#endif // SONGINFO_HPP
