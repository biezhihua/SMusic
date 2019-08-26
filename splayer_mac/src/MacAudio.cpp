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
            return NEGATIVE(S_NO_SDL_INIT);
        }

        Uint32 windowFlags = SDL_WINDOW_HIDDEN;

        options->videoWidth = 0;
        options->videoHeight = 0;

        window = SDL_CreateWindow(options->videoTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, options->videoWidth, options->videoHeight, windowFlags);

        if (window == nullptr) {
            ALOGE(MAC_AUDIO_TAG, "%s create sdl window fail: %s", __func__, SDL_GetError());
            destroy();
            return NEGATIVE(S_NO_SDL_CREATE_WINDOW);
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

int MacAudio::openAudio(int64_t wantedChannelLayout, int wantedNbChannels, int wantedSampleRate, AudioParams *audioHwParams) {

    static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};

    SDL_AudioSpec wantedSpec, spec;
    const char *env;

    int nextSampleRateIdx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

    env = SDL_getenv("SDL_AUDIO_CHANNELS");

    if (env) {
        wantedNbChannels = atoi(env);
        wantedChannelLayout = av_get_default_channel_layout(wantedNbChannels);
    }
    if (!wantedChannelLayout || wantedNbChannels != av_get_channel_layout_nb_channels(wantedChannelLayout)) {
        wantedChannelLayout = av_get_default_channel_layout(wantedNbChannels);
        wantedChannelLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    wantedNbChannels = av_get_channel_layout_nb_channels(wantedChannelLayout);
    wantedSpec.channels = wantedNbChannels;
    wantedSpec.freq = wantedSampleRate;

    if (wantedSpec.freq <= 0 || wantedSpec.channels <= 0) {
        ALOGE(MAC_AUDIO_TAG, "%s Invalid sample rate or channel count!", __func__);
        return NEGATIVE(S_INVALID_SAMPLE_RATE_OR_CHANNEL_COUNT);
    }

    while (nextSampleRateIdx && next_sample_rates[nextSampleRateIdx] >= wantedSpec.freq) {
        nextSampleRateIdx--;
    }

    wantedSpec.format = AUDIO_S16SYS;
    wantedSpec.silence = 0;
    wantedSpec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wantedSpec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wantedSpec.callback = sdlAudioCallback;
    wantedSpec.userdata = this;

    while (!(audioDev = SDL_OpenAudioDevice(nullptr, 0, &wantedSpec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {
        ALOGD(MAC_AUDIO_TAG, "%s SDL_OpenAudio (%d channels, %d Hz): %s", __func__, wantedSpec.channels, wantedSpec.freq, SDL_GetError());

        wantedSpec.channels = next_nb_channels[FFMIN(7, wantedSpec.channels)];
        if (!wantedSpec.channels) {
            wantedSpec.freq = next_sample_rates[nextSampleRateIdx--];
            wantedSpec.channels = wantedNbChannels;
            if (!wantedSpec.freq) {
                ALOGD(MAC_AUDIO_TAG, "%s No more combinations to try, audio open failed", __func__);
                return NEGATIVE(S_NOT_OPEN_AUDIO);
            }
        }
        wantedChannelLayout = av_get_default_channel_layout(wantedSpec.channels);
    }

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

    audioHwParams->fmt = AV_SAMPLE_FMT_S16;
    audioHwParams->freq = spec.freq;
    audioHwParams->channel_layout = wantedChannelLayout;
    audioHwParams->channels = spec.channels;
    audioHwParams->frame_size = av_samples_get_buffer_size(nullptr, audioHwParams->channels, 1, audioHwParams->fmt, 1);
    audioHwParams->bytes_per_sec = av_samples_get_buffer_size(nullptr, audioHwParams->channels, audioHwParams->freq, audioHwParams->fmt, 1);

    if (audioHwParams->bytes_per_sec <= 0 || audioHwParams->frame_size <= 0) {
        ALOGE(MAC_AUDIO_TAG, "%s av_samples_get_buffer_size failed", __func__);
        return NEGATIVE(S_NOT_SUPPORT_AUDIO_GET_BUFFER_SIZE);
    }

    return spec.size;
}

void MacAudio::audioCallback(Uint8 *stream, int len) {
    ALOGD(MAC_AUDIO_TAG, "%s stream = %p len = %d ", __func__, stream, len);

    if (!Audio::stream) {
        ALOGE(MAC_AUDIO_TAG, "%s stream is null", __func__);
        return;
    }

    if (!Audio::stream->getVideoState()) {
        ALOGE(MAC_AUDIO_TAG, "%s video state is null", __func__);
        return;
    }

    VideoState *is = Audio::stream->getVideoState();
    int audioSize, len1;

    audioCallbackTime = av_gettime_relative();

    while (len > 0) {
        if (is->audioBufIndex >= is->audioBufSize) {
            audioSize = audioDecodeFrame();
            if (audioSize < 0) {
                /* if error, just output silence */
                is->audioBuf = nullptr;
                is->audioBufSize = static_cast<unsigned int>(SDL_AUDIO_MIN_BUFFER_SIZE / is->audioTarget.frame_size * is->audioTarget.frame_size);
            } else {
                if (is->showMode != SHOW_MODE_VIDEO) {
                    update_sample_display((int16_t *) is->audioBuf, audioSize);
                }
                is->audioBufSize = static_cast<unsigned int>(audioSize);
            }
            is->audioBufIndex = 0;
        }
        len1 = is->audioBufSize - is->audioBufIndex;
        if (len1 > len)
            len1 = len;
        if (!is->audioMuted && is->audioBuf && is->audioVolume == SDL_MIX_MAXVOLUME) {
            memcpy(stream, (uint8_t *) is->audioBuf + is->audioBufIndex, len1);
        } else {
            memset(stream, 0, len1);
            if (!is->audioMuted && is->audioBuf) {
                SDL_MixAudioFormat(stream, (uint8_t *) is->audioBuf + is->audioBufIndex, AUDIO_S16SYS, len1, is->audioVolume);
            }
        }
        len -= len1;
        stream += len1;
        is->audioBufIndex += len1;
    }
    is->audio_write_buf_size = is->audioBufSize - is->audioBufIndex;
    /* Let's assume the audio driver that is used by SDL has two periods. */
    if (!isnan(is->audioClockTime)) {
        double pts = is->audioClockTime - (double) (2 * is->audioHwBufSize + is->audio_write_buf_size) / is->audioTarget.bytes_per_sec;
        int serial = is->audioClockSerial;
        double time = audioCallbackTime / 1000000.0;
        is->audioClock.setClockAt(pts, serial, time);
        is->exitClock.syncClockToSlave(&is->audioClock);
    }
}

void MacAudio::pauseAudio() {
    SDL_PauseAudioDevice(audioDev, 0);
}

void MacAudio::update_sample_display(short *samples, int samples_size) {

}



