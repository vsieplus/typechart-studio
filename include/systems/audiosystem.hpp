#ifndef AUDIOSYSTEM_HPP
#define AUDIOSYSTEM_HPP

#include <array>
#include <string>
#include <unordered_map>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
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
        bool loadMusic(std::string path);

        void playSound(std::string soundID);

        void startMusic(float startPosition = 0.f);
        void resumeMusic();
        void pauseMusic();
        void stopMusic();

        void setMusicVolume(float gain);
        void setSoundVolume(float gain);

        // calculate song pos, length in seconds
        float getMusicLength();
        float getSongPosition();

        bool isMusicPlaying() const;
        bool isMusicPaused() const;
    private:
        void initSoundSource(ALuint source, float pitch, float gain, std::array<float, 3> position,
                             std::array<float, 3> velocity, bool looping);

        float getBufferLength(ALuint bufid) const;

        void updateBufferStream(SDL_Window * window);

        static const int BUFFER_FRAMES = 8192;
        static const int NUM_BUFFERS = 4;

        static const int NUM_SOUND_SOURCES = 128;

        ALCdevice * soundDevice;
        ALCcontext * soundContext;

        ALuint soundBuffers[NUM_SOUND_SOURCES];
        ALuint soundSources[NUM_SOUND_SOURCES];

        ALuint musicBuffers[NUM_BUFFERS];
        ALuint musicSource;

        // for tracking time position of music
        float lastBufferPosition = 0.f;

        SNDFILE * sndfile = nullptr;
        SF_INFO sfInfo;

        float * membuf = nullptr;

        std::unordered_map<std::string, ALuint> soundBufferIDs;

        // Force to read float samples to avoid clipping issue
        ALenum musicFormat = AL_FORMAT_STEREO_FLOAT32;
};

#endif // ...