#ifndef AUDIOSYSTEM_HPP
#define AUDIOSYSTEM_HPP

#include <array>
#include <map>
#include <string>
#include <unordered_map>

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

/* Signature:
    - AudioComponent
*/
class AudioSystem {
    public:
        void initAudioSystem(SDL_Window * window);
        void quitAudioSystem();

        void update(SDL_Window * window);

        bool loadSound(std::string soundID, std::string path);
        int loadMusic(std::string path);

        void deactivateMusicSource(int sourceIdx);

        void playSound(std::string soundID);

        void startMusic(int sourceIdx, float startPosition = 0.f);
        void setMusicPosition(int sourceIdx, float position);
        void resumeMusic(int sourceIdx);
        void pauseMusic(int sourceIdx);
        void stopMusic(int sourceIdx);

        void setMusicVolume(float gain);
        void setSoundVolume(float gain);

        // calculate song pos, length in seconds
        float getMusicLength(int sourceIdx);
        float getSongPosition(int sourceIdx);

        bool isMusicPlaying(int sourceIdx) const;
        bool isMusicPaused(int sourceIdx) const;

        bool getStopMusicEarly(int sourceIdx) const;
        void setStopMusicEarly(int sourceIdx, bool stopMusicEarly);
        
        float getMusicStop(int sourceIdx) const;
        void setMusicStop(int sourceIdx, float musicStop);
    private:
        void initSoundSource(ALuint source, float pitch, float gain, std::array<float, 3> position,
                             std::array<float, 3> velocity, bool looping);

        float getBufferLength(ALuint bufid) const;

        void updateBufferStream(SDL_Window * window, int sourceIdx);

        static const int BUFFER_FRAMES = 8192;
        static const int NUM_BUFFERS = 4;

        static const int NUM_SOUND_SOURCES = 128;
        static const int NUM_MUSIC_SOURCES = 16;

        ALCdevice * soundDevice;
        ALCcontext * soundContext;

        ALuint soundBuffers[NUM_SOUND_SOURCES];
        ALuint soundSources[NUM_SOUND_SOURCES];

        ALuint musicBuffers[NUM_MUSIC_SOURCES][NUM_BUFFERS];
        ALuint musicSources[NUM_MUSIC_SOURCES];

        // for tracking time position of music
        float lastBufferPositions[NUM_MUSIC_SOURCES];
        float musicStops[NUM_MUSIC_SOURCES];

        bool stopMusicsEarly[NUM_MUSIC_SOURCES];

        SNDFILE * sndfiles[NUM_MUSIC_SOURCES];
        SF_INFO sfInfos[NUM_MUSIC_SOURCES];

        float * membufs[NUM_MUSIC_SOURCES];

        // SNDFILE * sndfile = nullptr;
        // SF_INFO sfInfo;

        // float * membuf = nullptr;

        std::unordered_map<std::string, ALuint> soundBufferIDs;

        std::map<int, bool> musicSourcesActive;

        // Force to read float samples to avoid clipping issue
#ifdef __APPLE__
        ALenum musicFormat = AL_FORMAT_STEREO16;
#else
        ALenum musicFormat = AL_FORMAT_STEREO_FLOAT32;
#endif
};

#endif // ...