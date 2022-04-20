#ifndef SONGINFO_HPP
#define SONGINFO_HPP

#include <string>

class SongInfo {
    SongInfo(std::string title, std::string artist, std::string musicFilename, std::string coverartFilename, std::string bpmtext, float musicPreviewStart, float musicPreviewStop) : 
        title(title), artist(artist), musicFilename(musicFilename), coverartFilename(coverartFilename), bpmtext(bpmtext) {}

    std::string title;
    std::string artist;
    std::string musicFilename;
    std::string coverartFilename;
    std::string bpmtext;

    float musicPreviewStart = 0.f;
    float musicPreviewStop = 0.f;
};

#endif // SONGINFO_HPP