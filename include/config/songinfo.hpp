#ifndef SONGINFO_HPP
#define SONGINFO_HPP

#include <filesystem>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

struct SongInfo {
    SongInfo() = default;
    SongInfo(std::string_view title, std::string_view artist, std::string_view genre, std::string_view bpmtext, std::string_view musicFilename,
        std::string_view coverartFilename, const fs::path & musicFilepath, const fs::path & coverartFilepath, float musicPreviewStart, float musicPreviewStop);

    bool loadSongInfo(const fs::path & songinfoPath, const fs::path & songinfoDir);
    void saveSongInfo(const fs::path & saveDir, bool initialSaved);

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

    int offsetMS { 0 };

    float musicPreviewStart { 0.0 };
    float musicPreviewStop { 0.0 };
};

#endif // SONGINFO_HPP
