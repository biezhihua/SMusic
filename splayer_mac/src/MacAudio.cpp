#include "MacAudio.h"

int MacAudio::create() {
    if (stream && options && !options->audioDisable && options->displayDisable && options->videoDisable) {

        unsigned int initFlags = SDL_INIT_AUDIO | SDL_INIT_TIMER;

        /* Try to work around an occasional ALSA buffer underflow issue when the
         * period size is NPOT due to ALSA resampling by forcing the buffer size. */
        if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE")) {
            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
        }

        if (SDL_Init(initFlags) != 0) {
            ALOGD(MAC_AUDIO_TAG, "%s init sdl fail code = %s", __func__, SDL_GetError());
            return NEGATIVE(S_NOT_SDL_INIT);
        }

        Uint32 windowFlags = SDL_WINDOW_HIDDEN;

        options->videoWidth = 0;
        options->videoHeight = 0;

        window = SDL_CreateWindow(options->videoTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  options->videoWidth, options->videoHeight, windowFlags);

        if (window == nullptr) {
            ALOGE(MAC_AUDIO_TAG, "%s create sdl window fail: %s", __func__, SDL_GetError());
            destroy();
            return NEGATIVE(S_NOT_SDL_CREATE_WINDOW);
        }
    }
    return POSITIVE;
}

int MacAudio::destroy() {
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
    options = nullptr;
    return POSITIVE;
}

/* prepare a new audio buffer */
static void sdlAudioCallback(void *opaque, Uint8 *stream, int len) {
    if (opaque != nullptr) {
        auto *audio = static_cast<MacAudio *>(opaque);
        audio->audioCallback(stream, len);
    }
}

int MacAudio::openAudio(int64_t wantedChannelLayout, int wantedNbChannels, int wantedSampleRate,
                        AudioParams *wantedAudioTarget) {

    ALOGD(MAC_AUDIO_TAG, "%s wantedChannelLayout = %lld wantedNbChannels = %d wantedSampleRate = %d", __func__,
          wantedChannelLayout,
          wantedNbChannels,
          wantedSampleRate);

    static const int nextNbChannels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    static const int nextSampleRates[] = {0, 44100, 48000, 96000, 192000};

    SDL_AudioSpec wantedSpec, spec;
    const char *env;

    int nextSampleRateIndex = FF_ARRAY_ELEMS(nextSampleRates) - 1;

    env = SDL_getenv("SDL_AUDIO_CHANNELS");

    if (env) {
        wantedNbChannels = atoi(env);
        wantedChannelLayout = av_get_default_channel_layout(wantedNbChannels);
    }

    int channelLayout = av_get_channel_layout_nb_channels(static_cast<uint64_t>(wantedChannelLayout));

    if (!wantedChannelLayout || wantedNbChannels != channelLayout) {
        wantedChannelLayout = av_get_default_channel_layout(wantedNbChannels);
        wantedChannelLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    wantedNbChannels = channelLayout;
    wantedSpec.channels = static_cast<Uint8>(wantedNbChannels);
    wantedSpec.freq = wantedSampleRate;

    if (wantedSpec.freq <= 0 || wantedSpec.channels <= 0) {
        ALOGE(MAC_AUDIO_TAG, "%s Invalid sample rate or channel count!", __func__);
        return NEGATIVE(S_SAMPLE_RATE_OR_CHANNEL_COUNT);
    }

    while (nextSampleRateIndex && nextSampleRates[nextSampleRateIndex] >= wantedSpec.freq) {
        nextSampleRateIndex--;
    }

    wantedSpec.format = AUDIO_S16SYS;
    wantedSpec.silence = 0;
    wantedSpec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE,
                               2 << av_log2(wantedSpec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wantedSpec.callback = sdlAudioCallback;
    wantedSpec.userdata = this;

    ALOGD(MAC_AUDIO_TAG, "%s SDL_OpenAudioDevice before channels = %d freq = %d format = %d silence = %d samples = %d",
          __func__,
          wantedSpec.channels,
          wantedSpec.freq,
          wantedSpec.format,
          wantedSpec.silence,
          wantedSpec.samples
    );

    while (!(audioDev = SDL_OpenAudioDevice(nullptr, 0, &wantedSpec, &spec,
                                            SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {

        ALOGD(MAC_AUDIO_TAG, "%s SDL_OpenAudio (%d channels, %d Hz): %s", __func__, wantedSpec.channels,
              wantedSpec.freq, SDL_GetError());

        wantedSpec.channels = (Uint8) nextNbChannels[FFMIN(7, wantedSpec.channels)];
        if (!wantedSpec.channels) {
            wantedSpec.freq = nextSampleRates[nextSampleRateIndex--];
            wantedSpec.channels = static_cast<Uint8>(wantedNbChannels);
            if (!wantedSpec.freq) {
                ALOGD(MAC_AUDIO_TAG, "%s No more combinations to try, audio open failed", __func__);
                return NEGATIVE(S_NOT_OPEN_AUDIO);
            }
        }
        wantedChannelLayout = av_get_default_channel_layout(wantedSpec.channels);
    }

    ALOGD(MAC_AUDIO_TAG,
          "%s SDL_OpenAudioDevice after channels = %d freq = %d format = %d silence = %d samples = %d size = %d",
          __func__,
          spec.channels,
          spec.freq,
          spec.format,
          spec.silence,
          spec.samples,
          spec.size
    );

    if (spec.format != AUDIO_S16SYS) {
        ALOGE(MAC_AUDIO_TAG, "%s SDL advised audio format %d is not supported!", __func__, spec.format);
        return NEGATIVE(S_NOT_SUPPORT_AUDIO_FORMAT);
    }

    if (spec.channels != wantedSpec.channels) {
        wantedChannelLayout = av_get_default_channel_layout(spec.channels);
        if (!wantedChannelLayout) {
            ALOGE(MAC_AUDIO_TAG, "%s SDL advised channel count %d is not supported!", __func__, spec.channels);
            return NEGATIVE(S_NOT_SUPPORT_AUDIO_CHANNEL_COUNT);
        }
    }

    wantedAudioTarget->sampleFormat = AV_SAMPLE_FMT_S16;
    wantedAudioTarget->sampleRate = spec.freq;
    wantedAudioTarget->channelLayout = wantedChannelLayout;
    wantedAudioTarget->channels = spec.channels;
    wantedAudioTarget->frameSize = av_samples_get_buffer_size(nullptr, wantedAudioTarget->channels, 1,
                                                              wantedAudioTarget->sampleFormat, 1);
    wantedAudioTarget->bytesPerSec = av_samples_get_buffer_size(nullptr, wantedAudioTarget->channels,
                                                                wantedAudioTarget->sampleRate,
                                                                wantedAudioTarget->sampleFormat, 1);

    if (wantedAudioTarget->bytesPerSec <= 0 || wantedAudioTarget->frameSize <= 0) {
        ALOGE(MAC_AUDIO_TAG, "%s av_samples_get_buffer_size failed", __func__);
        return NEGATIVE(S_NOT_SUPPORT_AUDIO_GET_BUFFER_SIZE);
    }

    return spec.size;
}


void MacAudio::pauseAudio() {
    SDL_PauseAudioDevice(audioDev, 0);
}

void MacAudio::maxAudio(uint8_t *stream, uint8_t *buf, int index, int length, int volume) {
    SDL_MixAudioFormat(stream, buf + index, AUDIO_S16SYS, static_cast<Uint32>(length), volume);
}




