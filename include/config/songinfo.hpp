#ifndef SONGINFO_HPP
#define SONGINFO_HPP

#include <string>

struct SongInfo {
    SongInfo(std::string title, std::string artist, std::string bpmtext, std::string musicFilename, 
        std::string coverartFilename, std::string musicFilepath, std::string coverartFilepath, float musicPreviewStart, float musicPreviewStop) : 
        title(title), artist(artist), bpmtext(bpmtext), musicFilename(musicFilename), coverartFilename(coverartFilename),
        musicFilepath(musicFilepath), coverartFilepath(coverartFilepath),
        musicPreviewStart(musicPreviewStart), musicPreviewStop(musicPreviewStop) {}

    void saveSonginfo(std::string savePath);

    std::string title;
    std::string artist;
    std::string bpmtext;

    std::string musicFilename;
    std::string coverartFilename;

    // if user wishes to copy files when saving config
    std::string musicFilepath;
    std::string coverartFilepath;

    std::string saveDir = "";

    float musicPreviewStart = 0.f;
    float musicPreviewStop = 0.f;
};

#endif // SONGINFO_HPP