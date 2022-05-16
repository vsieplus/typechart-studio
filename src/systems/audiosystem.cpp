#include <limits.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "systems/audiosystem.hpp"

void AudioSystem::initAudioSystem(SDL_Window * window) {
    // Initialize sound device
    soundDevice = alcOpenDevice(nullptr);
    if(!soundDevice) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio system initialization failure", "Failed to load sound device", window);
        return;
    }

    soundContext = alcCreateContext(soundDevice, nullptr);
    if(!soundContext) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio system initialization failure", "Failed to get sound context", window);
        return;
    }

    if(!alcMakeContextCurrent(soundContext)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio system initialization failure", "Failed to set current context", window);
        return;
    }

    // Get name of the device we just loaded
    const ALCchar * deviceName = nullptr;
    if(alcIsExtensionPresent(soundDevice, "ALC_ENUMERATE_ALL_EXT")) {
        deviceName = alcGetString(soundDevice, ALC_ALL_DEVICES_SPECIFIER);
    }

    if(!deviceName || alcGetError(soundDevice) != ALC_NO_ERROR) {
        deviceName = alcGetString(soundDevice, ALC_DEVICE_SPECIFIER);
    }

    printf("Using sound device %s\n", deviceName);

    // setup listener position, velocity
    alListener3f(AL_POSITION, 0, 0, 1.f);
    alListener3f(AL_VELOCITY, 0, 0, 0);

    // listener look direction, up direction
    ALfloat listenerOrientation[] = { 0.f, 0.f, -1.f, 0.f, 1.f, 0.f };
    alListenerfv(AL_ORIENTATION, listenerOrientation);

    // Setup default sound sources
    for(int i = 0; i < NUM_SOUND_SOURCES; i++) {
        alGenSources(1, &soundSources[i]);
        initSoundSource(soundSources[i], 1.f, 1.f, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, false);

        soundBuffers[i] = 0;
    }

    // Setup music sources
    for(int i = 0; i < NUM_MUSIC_SOURCES; i++) {
        alGenSources(1, &musicSources[i]);
        alGenBuffers(NUM_BUFFERS, musicBuffers[i]);
        initSoundSource(musicSources[i], 1.f, 1.f, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, false);

        sndfiles[i] = nullptr;
        membufs[i] = nullptr;

        stopMusicsEarly[i] = false;
        musicStops[i] = 0.f;
        lastBufferPositions[i] = 0.f;

        memset(&sfInfos[i], 0, sizeof(SF_INFO));

        musicSourcesActive.insert({ i, false });
    }
}

void AudioSystem::initSoundSource(ALuint source, float pitch, float gain, std::array<float, 3> position,
                                  std::array<float, 3> velocity, bool looping)
{
    alSourcef(source, AL_PITCH, pitch);
    alSourcef(source, AL_GAIN, gain);
    alSource3f(source, AL_POSITION, position[0], position[1], position[2]);
    alSource3f(source, AL_VELOCITY, velocity[0], velocity[1], velocity[2]);
    alSourcei(source, AL_LOOPING, looping);
}

void AudioSystem::quitAudioSystem() {
    for(int i = 0; i < NUM_SOUND_SOURCES; i++) {
        alSourceStop(soundSources[i]);
        alSourcei(soundSources[i], AL_BUFFER, 0);
    }

    for(int i = 0; i < NUM_MUSIC_SOURCES; i++) {
        alSourceStop(musicSources[i]);
        alSourcei(musicSources[i], AL_BUFFER, 0);

        if(sndfiles[i]) {
            sf_close(sndfiles[i]);
            sndfiles[i] = nullptr;
        }

        if(membufs[i]) {
            free(membufs[i]);
            membufs[i] = nullptr;
        }
    }
    
    alDeleteSources(NUM_SOUND_SOURCES, soundSources);
    for(auto & [soundID, soundBuffer] : soundBufferIDs) {
        if(soundBuffer && alIsBuffer(soundBuffer)) {
            alDeleteBuffers(1, &soundBuffer);
        }
    }

    alDeleteSources(NUM_MUSIC_SOURCES, musicSources);

    for(int i = 0; i < NUM_MUSIC_SOURCES; i++)
        alDeleteBuffers(NUM_BUFFERS, musicBuffers[i]);

    if(!alcMakeContextCurrent(nullptr)) {
        printf("Failed to reset context to null");
    }

    alcDestroyContext(soundContext);
    if(alcGetError(soundDevice) != AL_NO_ERROR) {
        printf("Failed to destroy sound context");
    }

    if(!alcCloseDevice(soundDevice)) {
        printf("Failed to close sound device");
    }
}

void AudioSystem::update(SDL_Window * window) {
    for(auto & [sourceIdx, active] : musicSourcesActive) {
        if(active && isMusicPlaying(sourceIdx)) {
            updateBufferStream(window, sourceIdx);

            if(stopMusicsEarly[sourceIdx] && getSongPosition(sourceIdx) > musicStops[sourceIdx]) {
                stopMusic(sourceIdx);
            }
        }

    }
}

bool AudioSystem::loadSound(std::string soundID, std::string path) {
    // adapted from https://github.com/kcat/openal-soft/blob/master/examples/alplay.c
    ALenum err, format;
    ALuint buffer;
    SNDFILE * soundSndfile;
    SF_INFO soundSfinfo;
    short * soundMembuf;
    sf_count_t num_frames;
    ALsizei num_bytes;

    // Open the audio file and check that it's usable
    soundSndfile = sf_open(path.c_str(), SFM_READ, &soundSfinfo);
    if(!soundSndfile) {
        return false;
    }

    if(soundSfinfo.frames < 1 || soundSfinfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/soundSfinfo.channels) {
        return false;
    }

    /* Get the sound format, and figure out the OpenAL format */
    format = AL_NONE;
    if(soundSfinfo.channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if(soundSfinfo.channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else if(soundSfinfo.channels == 3) {
        if(sf_command(soundSndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT2D_16;
    } else if(soundSfinfo.channels == 4) {
        if(sf_command(soundSndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT3D_16;
    }

    if(!format) {
        return false;
    }

    /* Decode the whole audio file to a buffer. */
    soundMembuf = static_cast<short *>(malloc((size_t)(soundSfinfo.frames * soundSfinfo.channels) * sizeof(short)));

    num_frames = sf_readf_short(soundSndfile, soundMembuf, soundSfinfo.frames);
    if(num_frames < 1) {
        free(soundMembuf);
        sf_close(soundSndfile);

        return false;
    }

    num_bytes = (ALsizei)(num_frames * soundSfinfo.channels) * (ALsizei)sizeof(short);

    /* Buffer the audio data into a new buffer object, then free the data and
     * close the file.
     */
    buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, soundMembuf, num_bytes, soundSfinfo.samplerate);

    free(soundMembuf);
    sf_close(soundSndfile);

    /* Check if an error occured, and clean up if so. */
    err = alGetError();
    if(err != AL_NO_ERROR) {
        if(buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);

        return false;
    }

    if(soundBufferIDs.find(soundID) == soundBufferIDs.end()) {
        soundBufferIDs.insert({ soundID, buffer });
    } else {
        alDeleteBuffers(1, &buffer);
    }

    return true;
}

int AudioSystem::loadMusic(std::string path) {
    int nextIdx = -1;
    for(auto & [sourceIdx, active] : musicSourcesActive) {
        if(!active) {
            nextIdx = sourceIdx;
            break;
        }
    }

    if(nextIdx >= 0) {
        std::size_t frameSize;
        if(sndfiles[nextIdx]) {
            sf_close(sndfiles[nextIdx]);
        }

        memset(&sfInfos[nextIdx], 0, sizeof(SF_INFO));
        sndfiles[nextIdx] = sf_open(path.c_str(), SFM_READ, &sfInfos[nextIdx]);
        
        if(!sndfiles[nextIdx]) {
            return -1;
        }

        frameSize = ((size_t)BUFFER_FRAMES * (size_t)sfInfos[nextIdx].channels) * sizeof(float);

        if(membufs[nextIdx]) {
            free(membufs[nextIdx]);
        }

        membufs[nextIdx] = static_cast<float*>(malloc(frameSize));
        musicSourcesActive[nextIdx] = true;
    }

    return nextIdx;
}

void AudioSystem::deactivateMusicSource(int sourceIdx) {
    if(sourceIdx < NUM_MUSIC_SOURCES && sourceIdx >= 0 && musicSourcesActive.at(sourceIdx)) {
        sf_close(sndfiles[sourceIdx]);
        free(membufs[sourceIdx]);

        memset(&sfInfos[sourceIdx], 0, sizeof(SF_INFO));

        musicStops[sourceIdx] = 0.f;
        stopMusicsEarly[sourceIdx] = false;
        sndfiles[sourceIdx] = nullptr;
        membufs[sourceIdx] = nullptr;

        musicSourcesActive[sourceIdx] = false;
    }
}

void AudioSystem::playSound(std::string soundID) {
    if(soundBufferIDs.find(soundID) != soundBufferIDs.end()) {
        auto bufferID = soundBufferIDs.at(soundID);

        if(!(bufferID && alIsBuffer(bufferID))) {
            return;
        }

        // find the next available sound buffer, and set it
        for(int i = 0; i < NUM_SOUND_SOURCES; i++) {
            ALint state;
            alGetSourcei(soundSources[i], AL_SOURCE_STATE, &state);

            if(state != AL_PLAYING) {
                if(bufferID != soundBuffers[i]) {
                    soundBuffers[i] = bufferID;
                    alSourcei(soundSources[i], AL_BUFFER, (ALint) soundBuffers[i]);
                }

                alSourcePlay(soundSources[i]);
                return;
            }
        }
    }
}

void AudioSystem::startMusic(int sourceIdx, float startPosition) {
    setMusicPosition(sourceIdx, startPosition);

    // start playback
    if(sourceIdx < NUM_MUSIC_SOURCES)
        alSourcePlay(musicSources[sourceIdx]);
}

void AudioSystem::setMusicPosition(int sourceIdx, float position) {
    if(!(sourceIdx < NUM_MUSIC_SOURCES))
        return;

    alSourceRewind(musicSources[sourceIdx]);
    alSourcei(musicSources[sourceIdx], AL_BUFFER, 0);

    sf_count_t numFramesToSeek = (sf_count_t)((position / getMusicLength(sourceIdx)) * sfInfos[sourceIdx].frames);
    sf_seek(sndfiles[sourceIdx], numFramesToSeek, SEEK_SET);

    lastBufferPositions[sourceIdx] = position;

    ALsizei b;
    for(b = 0; b < NUM_BUFFERS; b++) {
        sf_count_t sndLen = sf_readf_float(sndfiles[sourceIdx], membufs[sourceIdx], BUFFER_FRAMES);
        if(sndLen < 1) break;

        sndLen *= sfInfos[sourceIdx].channels * (sf_count_t) sizeof(float);
        alBufferData(musicBuffers[sourceIdx][b], musicFormat, membufs[sourceIdx], (ALsizei)sndLen, sfInfos[sourceIdx].samplerate);
    }

    alSourceQueueBuffers(musicSources[sourceIdx], b, musicBuffers[sourceIdx]);
    
    alSourcePlay(musicSources[sourceIdx]);
    alSourcePause(musicSources[sourceIdx]);
}

float AudioSystem::getMusicLength(int sourceIdx) {
    return sourceIdx < NUM_MUSIC_SOURCES ? (float) sfInfos[sourceIdx].frames / (float) sfInfos[sourceIdx].samplerate : 0.f;
}

void AudioSystem::resumeMusic(int sourceIdx) {
    if(isMusicPaused(sourceIdx)) {
        alSourcePlay(musicSources[sourceIdx]);
    }
}

void AudioSystem::pauseMusic(int sourceIdx) {
    if(isMusicPlaying(sourceIdx)) {
        alSourcePause(musicSources[sourceIdx]);
    }
}

void AudioSystem::stopMusic(int sourceIdx) {
    if(sourceIdx < NUM_MUSIC_SOURCES) {
        lastBufferPositions[sourceIdx] = 0;
        alSourceStop(musicSources[sourceIdx]);
    }
}

bool AudioSystem::isMusicPlaying(int sourceIdx) const {
    if(!(sourceIdx < NUM_MUSIC_SOURCES))
        return false;

    ALint state;
    alGetSourcei(musicSources[sourceIdx], AL_SOURCE_STATE, &state);
    
    return (state == AL_PLAYING && alGetError() == AL_NO_ERROR);
}

bool AudioSystem::isMusicPaused(int sourceIdx) const {
    if(!(sourceIdx < NUM_MUSIC_SOURCES))
        return false;

    ALint state;
    alGetSourcei(musicSources[sourceIdx], AL_SOURCE_STATE, &state);
    
    return (state == AL_PAUSED && alGetError() == AL_NO_ERROR);
}

void AudioSystem::updateBufferStream(SDL_Window * window, int sourceIdx) {
    if(!(sourceIdx < NUM_MUSIC_SOURCES))
        return;

    ALint processed, state;

	/* Get source info */
	alGetSourcei(musicSources[sourceIdx], AL_SOURCE_STATE, &state);
	alGetSourcei(musicSources[sourceIdx], AL_BUFFERS_PROCESSED, &processed);

	/* Unqueue and handle each processed buffer */
	while(processed > 0) {
		ALuint bufid;
		sf_count_t slen;

		alSourceUnqueueBuffers(musicSources[sourceIdx], 1, &bufid);
		processed--;

        lastBufferPositions[sourceIdx] += getBufferLength(bufid);
        //printf("Unqueued buffer(s), lastBufferPosition: %.4f\n", lastBufferPosition);

		/* Read the next chunk of data, refill the buffer, and queue it
		 * back on the source */
		slen = sf_readf_float(sndfiles[sourceIdx], membufs[sourceIdx], BUFFER_FRAMES);
		if(slen > 0) {
			slen *= sfInfos[sourceIdx].channels * (sf_count_t)sizeof(float);
			alBufferData(bufid, musicFormat, membufs[sourceIdx], (ALsizei)slen, sfInfos[sourceIdx].samplerate);
			alSourceQueueBuffers(musicSources[sourceIdx], 1, &bufid);

            //printf("slen: %.4ld\n", slen);
		}

		if(alGetError() != AL_NO_ERROR) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Music playback error", "Error buffering music data", window);
		}
	}

	/* Make sure the source hasn't underrun */
	if(state != AL_PLAYING && state != AL_PAUSED) {
		ALint queued;

		/* If no buffers are queued, playback is finished */
		alGetSourcei(musicSources[sourceIdx], AL_BUFFERS_QUEUED, &queued);
		if (queued == 0)
			return;

		alSourcePlay(musicSources[sourceIdx]);
	}
}

float AudioSystem::getBufferLength(ALuint bufid) const {
    ALint bytesize, channels, bits;

    alGetBufferi(bufid, AL_SIZE, &bytesize);
    alGetBufferi(bufid, AL_CHANNELS, &channels);
    alGetBufferi(bufid, AL_BITS, &bits);

    int sampleLength = (bytesize * 8) / (channels * bits);

    ALint frequency;
    alGetBufferi(bufid, AL_FREQUENCY, &frequency);

    float secLength = (float) sampleLength / (float) frequency;

    return secLength;
}

float AudioSystem::getSongPosition(int sourceIdx) {
    if(sourceIdx < NUM_MUSIC_SOURCES)
        return 0.f;

    // calculate position from the current buffer
    float songPosSec;
    alGetSourcef(musicSources[sourceIdx], AL_SEC_OFFSET, &songPosSec);

    return lastBufferPositions[sourceIdx] + songPosSec;
}

void AudioSystem::setStopMusicEarly(int sourceIdx, bool stopMusicEarly) {
    if(sourceIdx < NUM_MUSIC_SOURCES) {
        stopMusicsEarly[sourceIdx] = stopMusicEarly;
    }
}

bool AudioSystem::getStopMusicEarly(int sourceIdx) const {
    return sourceIdx < NUM_MUSIC_SOURCES ? stopMusicsEarly[sourceIdx] : false;
}

void AudioSystem::setMusicStop(int sourceIdx, float musicStop) {
    if(sourceIdx < NUM_MUSIC_SOURCES) {
        musicStops[sourceIdx] = musicStop;
    }
}

float AudioSystem::getMusicStop(int sourceIdx) const {
    return sourceIdx < NUM_MUSIC_SOURCES ? musicStops[sourceIdx] : 0;
}

void AudioSystem::setMusicVolume(float gain) {
    if(gain < 0) {
        gain = 0;
    } else if (gain > 1) {
        gain = 1;
    }

    for(int i = 0; i < NUM_MUSIC_SOURCES; i++) {
        alSourcef(musicSources[i], AL_GAIN, gain);
    }
}

void AudioSystem::setSoundVolume(float gain) {
    for(int i = 0; i < NUM_SOUND_SOURCES; i++) {
        auto source = soundSources[i];
        alSourcef(source, AL_GAIN, gain);
    }
}