#ifndef AUDIOSYSTEM_HPP
#define AUDIOSYSTEM_HPP

#include <array>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>

#include <filesystem>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <OpenAL/MacOSX_OALExtensions.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#endif

#include <sndfile.h>

#include <SDL2/SDL.h>

namespace fs = std::filesystem;

/* Signature:
    - AudioComponent
*/
class AudioSystem {
    public:
        void initAudioSystem(SDL_Window * window);
        void quitAudioSystem();

        void update(SDL_Window * window);

        bool loadSound(std::string_view soundID, const fs::path & path);
        int loadMusic(const fs::path & path);

        void deactivateMusicSource(int sourceIdx);

        void playSound(std::string_view soundID);

        void startMusic(int sourceIdx, float startPosition = 0.f);
        void setMusicPosition(int sourceIdx, float position);
        void resumeMusic(int sourceIdx) const;
        void pauseMusic(int sourceIdx) const;
        void stopMusic(int sourceIdx);

        void setMusicVolume(float gain);
        void setSoundVolume(float gain);

        // calculate song pos, length in seconds
        float getMusicLength(int sourceIdx) const;
        float getSongPosition(int sourceIdx) const;

        bool isMusicPlaying(int sourceIdx) const;
        bool isMusicPaused(int sourceIdx) const;

        bool getStopMusicEarly(int sourceIdx) const;
        void setStopMusicEarly(int sourceIdx, bool stopMusicEarly);
        
        float getMusicStop(int sourceIdx) const;
        void setMusicStop(int sourceIdx, float musicStop);
    private:
        void initSoundSource(ALuint source, float pitch, float gain, std::array<float, 3> position, std::array<float, 3> velocity, bool looping) const;

        float getBufferLength(ALuint bufid) const;

        void updateBufferStream(SDL_Window * window, int sourceIdx);

        static const int BUFFER_FRAMES = 8192;
        static const int NUM_BUFFERS = 4;

        static const int NUM_SOUND_SOURCES = 128;
        static const int NUM_MUSIC_SOURCES = 16;

        ALCdevice * soundDevice;
        ALCcontext * soundContext;

        std::array<ALuint, NUM_SOUND_SOURCES> soundBuffers;
        std::array<ALuint, NUM_SOUND_SOURCES> soundSources;

        std::array<std::array<ALuint, NUM_MUSIC_SOURCES>, NUM_BUFFERS> musicBuffers;
        std::array<ALuint, NUM_MUSIC_SOURCES> musicSources;

        // for tracking time position of music
        std::array<float, NUM_MUSIC_SOURCES> lastBufferPositions;
        std::array<float, NUM_MUSIC_SOURCES> musicStops;
        std::array<bool, NUM_MUSIC_SOURCES> stopMusicsEarly;

        std::array<SNDFILE *, NUM_MUSIC_SOURCES> sndfiles;
        std::array<SF_INFO, NUM_MUSIC_SOURCES> sfInfos;
        std::array<float *, NUM_MUSIC_SOURCES> membufs;

        std::unordered_map<std::string_view, ALuint> soundBufferIDs;

        std::map<int, bool> musicSourcesActive;

        // Force to read float samples to avoid clipping issue
#ifdef __APPLE__
        ALenum musicFormat = AL_FORMAT_STEREO16;
#else
        ALenum musicFormat = AL_FORMAT_STEREO_FLOAT32;
#endif
};

#endif // AUDIOSYSTEM_HPP
