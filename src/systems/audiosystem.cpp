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

    // Setup music source
    alGenSources(1, &musicSource);
    alGenBuffers(NUM_BUFFERS, musicBuffers);
    initSoundSource(musicSource, 1.f, 1.f, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, false);
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

    alSourceStop(musicSource);
    alSourcei(musicSource, AL_BUFFER, 0);

    if(sndfile) {
        sf_close(sndfile);
        sndfile = nullptr;
    }

    if(membuf) {
        free(membuf);
        membuf = nullptr;
    }
    
    alDeleteSources(NUM_SOUND_SOURCES, soundSources);
    for(auto & [soundID, soundBuffer] : soundBufferIDs) {
        if(soundBuffer && alIsBuffer(soundBuffer)) {
            alDeleteBuffers(1, &soundBuffer);
        }
    }

    alDeleteSources(1, &musicSource);
    alDeleteBuffers(NUM_BUFFERS, musicBuffers);

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
    if(isMusicPlaying()) {
        updateBufferStream(window);
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
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT2D_16;
    } else if(soundSfinfo.channels == 4) {
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
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

bool AudioSystem::loadMusic(std::string path) {
    std::size_t frameSize;

    if(sndfile) {
        sf_close(sndfile);
    }

    memset(&sfInfo, 0, sizeof(sfInfo));
	sndfile = sf_open(path.c_str(), SFM_READ, &sfInfo);
	
    if(!sndfile) {
        return false;
	}

	frameSize = ((size_t)BUFFER_FRAMES * (size_t)sfInfo.channels) * sizeof(float);

    if(membuf) {
        free(membuf);
    }

	membuf = static_cast<float*>(malloc(frameSize));

    return true;
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

void AudioSystem::startMusic(float startPosition) {
    alSourceRewind(musicSource);
    alSourcei(musicSource, AL_BUFFER, 0);

    ALsizei b;

    sf_count_t numFramesToSeek = (sf_count_t)((startPosition / getMusicLength()) * sfInfo.frames);
    sf_seek(sndfile, numFramesToSeek, SEEK_SET);

    lastBufferPosition = startPosition;

    for(b = 0; b < NUM_BUFFERS; b++) {
        sf_count_t sndLen = sf_readf_float(sndfile, membuf, BUFFER_FRAMES);
        if(sndLen < 1) break;

        sndLen *= sfInfo.channels * (sf_count_t) sizeof(float);
        alBufferData(musicBuffers[b], musicFormat, membuf, (ALsizei)sndLen, sfInfo.samplerate);
    }

    // start playback
    alSourceQueueBuffers(musicSource, b, musicBuffers);
    alSourcePlay(musicSource);
}

float AudioSystem::getMusicLength() {
    return (float) sfInfo.frames / (float) sfInfo.samplerate;
}

void AudioSystem::resumeMusic() {
    if(isMusicPaused()) {
        alSourcePlay(musicSource);
    }
}

void AudioSystem::pauseMusic() {
    if(isMusicPlaying()) {
        alSourcePause(musicSource);
    }
}

void AudioSystem::stopMusic() {
    lastBufferPosition = 0;
    alSourceStop(musicSource);
}

bool AudioSystem::isMusicPlaying() const {
    ALint state;
    alGetSourcei(musicSource, AL_SOURCE_STATE, &state);
    
    return (state == AL_PLAYING && alGetError() == AL_NO_ERROR);
}

bool AudioSystem::isMusicPaused() const {
    ALint state;
    alGetSourcei(musicSource, AL_SOURCE_STATE, &state);
    
    return (state == AL_PAUSED && alGetError() == AL_NO_ERROR);
}

void AudioSystem::updateBufferStream(SDL_Window * window) {
    ALint processed, state;

	/* Get source info */
	alGetSourcei(musicSource, AL_SOURCE_STATE, &state);
	alGetSourcei(musicSource, AL_BUFFERS_PROCESSED, &processed);

	/* Unqueue and handle each processed buffer */
	while(processed > 0) {
		ALuint bufid;
		sf_count_t slen;

		alSourceUnqueueBuffers(musicSource, 1, &bufid);
		processed--;

        lastBufferPosition += getBufferLength(bufid);
        //printf("Unqueued buffer(s), lastBufferPosition: %.4f\n", lastBufferPosition);

		/* Read the next chunk of data, refill the buffer, and queue it
		 * back on the source */
		slen = sf_readf_float(sndfile, membuf, BUFFER_FRAMES);
		if(slen > 0) {
			slen *= sfInfo.channels * (sf_count_t)sizeof(float);
			alBufferData(bufid, musicFormat, membuf, (ALsizei)slen, sfInfo.samplerate);
			alSourceQueueBuffers(musicSource, 1, &bufid);

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
		alGetSourcei(musicSource, AL_BUFFERS_QUEUED, &queued);
		if (queued == 0)
			return;

		alSourcePlay(musicSource);
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

float AudioSystem::getSongPosition() {
    // calculate position from the current buffer
    float songPosSec;
    alGetSourcef(musicSource, AL_SEC_OFFSET, &songPosSec);

    return lastBufferPosition + songPosSec;
}

void AudioSystem::setMusicVolume(float gain) {
    if(gain < 0) {
        gain = 0;
    } else if (gain > 1) {
        gain = 1;
    }

    alSourcef(musicSource, AL_GAIN, gain);
}

void AudioSystem::setSoundVolume(float gain) {
    for(int i = 0; i < NUM_SOUND_SOURCES; i++) {
        auto source = soundSources[i];
        alSourcef(source, AL_GAIN, gain);
    }
}