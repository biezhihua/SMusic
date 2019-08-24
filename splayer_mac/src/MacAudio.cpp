#include "MacAudio.h"

int MacAudio::create() {
    return POSITIVE;
}

int MacAudio::destroy() {
    return POSITIVE;
}

/* prepare a new audio buffer */
static void sdlAudioCallback(void *opaque, Uint8 *stream, int len) {
    if (opaque != nullptr) {
        auto *audio = static_cast<MacAudio *>(opaque);
        audio->audioCallback(stream, len);
    }
}

int MacAudio::openAudio(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params) {
    Audio::openAudio(wanted_channel_layout, wanted_nb_channels, wanted_sample_rate, audio_hw_params);

    static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};

    SDL_AudioSpec wanted_spec, spec;
    const char *env;

    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

    env = SDL_getenv("SDL_AUDIO_CHANNELS");

    if (env) {
        wanted_nb_channels = atoi(env);
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
    }
    if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;

    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        ALOGE(MAC_AUDIO_TAG, "%s Invalid sample rate or channel count!", __func__);
        return NEGATIVE(S_INVALID_SAMPLE_RATE_OR_CHANNEL_COUNT);
    }

    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq) {
        next_sample_rate_idx--;
    }

    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = sdlAudioCallback;
    wanted_spec.userdata = this;

    while (!(audioDev = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {
        ALOGD(MAC_AUDIO_TAG, "%s SDL_OpenAudio (%d channels, %d Hz): %s", __func__, wanted_spec.channels, wanted_spec.freq, SDL_GetError());

        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq) {
                ALOGD(MAC_AUDIO_TAG, "%s No more combinations to try, audio open failed", __func__);
                return NEGATIVE(S_NOT_OPEN_AUDIO);
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }

    if (spec.format != AUDIO_S16SYS) {
        ALOGE(MAC_AUDIO_TAG, "%s SDL advised audio format %d is not supported!", __func__, spec.format);
        return NEGATIVE(S_NOT_SUPPORT_AUDIO_FORMAT);
    }

    if (spec.channels != wanted_spec.channels) {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            ALOGE(MAC_AUDIO_TAG, "%s SDL advised channel count %d is not supported!", __func__, spec.channels);
            return NEGATIVE(S_NOT_SUPPORT_AUDIO_CHANNEL_COUNT);
        }
    }

    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;
    audio_hw_params->channel_layout = wanted_channel_layout;
    audio_hw_params->channels = spec.channels;
    audio_hw_params->frame_size = av_samples_get_buffer_size(nullptr, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(nullptr, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);

    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        ALOGE(MAC_AUDIO_TAG, "%s av_samples_get_buffer_size failed", __func__);
        return NEGATIVE(S_NOT_SUPPORT_AUDIO_GET_BUFFER_SIZE);
    }

    return spec.size;
}

void MacAudio::audioCallback(Uint8 *stream, int len) {
    ALOGD(MAC_AUDIO_TAG, "%s stream = %p len = %d ", __func__, stream, len);
}
