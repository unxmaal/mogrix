/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  IRIX dmedia audio backend, adapted from sgug-rse SDL2 patch.
  Updated for SDL2 2.30.x API.

  Supports both old IRIX 5.x AL API and new 6.5+ API.
*/

#include "../../SDL_internal.h"

#if SDL_AUDIO_DRIVER_DMEDIA

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "SDL_irixaudio.h"

#include <errno.h>

#ifndef AL_RESOURCE /* test whether we use the old IRIX audio libraries */
#define OLD_IRIX_AUDIO
#define alClosePort(x) ALcloseport(x)
#define alFreeConfig(x) ALfreeconfig(x)
#define alGetFillable(x) ALgetfillable(x)
#define alNewConfig() ALnewconfig()
#define alOpenPort(x,y,z) ALopenport(x,y,z)
#define alSetChannels(x,y) ALsetchannels(x,y)
#define alSetQueueSize(x,y) ALsetqueuesize(x,y)
#define alSetSampFmt(x,y) ALsetsampfmt(x,y)
#define alSetWidth(x,y) ALsetwidth(x,y)
#endif

static void IRIXAUDIO_WaitDevice(_THIS)
{
    Sint32 timeleft;

    timeleft = _this->spec.samples - alGetFillable(_this->hidden->audio_port);
    if (timeleft > 0) {
        timeleft /= (_this->spec.freq / 1000);
        SDL_Delay((Uint32)timeleft);
    }
}

static void IRIXAUDIO_PlayDevice(_THIS)
{
    if (alWriteFrames(_this->hidden->audio_port, _this->hidden->mixbuf,
                      _this->spec.samples) < 0) {
        SDL_OpenedAudioDeviceDisconnected(_this);
    }
}

static Uint8 *IRIXAUDIO_GetDeviceBuf(_THIS)
{
    return _this->hidden->mixbuf;
}

static void IRIXAUDIO_CloseDevice(_THIS)
{
    if (_this->hidden) {
        if (_this->hidden->mixbuf != NULL) {
            SDL_free(_this->hidden->mixbuf);
        }
        if (_this->hidden->audio_port != NULL) {
            alClosePort(_this->hidden->audio_port);
        }
        SDL_free(_this->hidden);
    }
}

static int IRIXAUDIO_OpenDevice(_THIS, const char *devname)
{
    SDL_AudioFormat test_format = SDL_FirstAudioFormat(_this->spec.format);
    long width = 0;
    long fmt = 0;
    int valid = 0;

    _this->hidden = (struct SDL_PrivateAudioData *)
        SDL_malloc(sizeof(*_this->hidden));
    if (!_this->hidden) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(_this->hidden);

#ifdef OLD_IRIX_AUDIO
    {
        long audio_param[2];
        audio_param[0] = AL_OUTPUT_RATE;
        audio_param[1] = _this->spec.freq;
        valid = (ALsetparams(AL_DEFAULT_DEVICE, audio_param, 2) < 0);
    }
#else
    {
        ALpv audio_param;
        audio_param.param = AL_RATE;
        audio_param.value.i = _this->spec.freq;
        valid = (alSetParams(AL_DEFAULT_OUTPUT, &audio_param, 1) < 0);
    }
#endif

    while ((!valid) && (test_format)) {
        valid = 1;
        _this->spec.format = test_format;

        switch (test_format) {
            case AUDIO_S8:
                width = AL_SAMPLE_8;
                fmt = AL_SAMPFMT_TWOSCOMP;
                break;

            case AUDIO_S16SYS:
                width = AL_SAMPLE_16;
                fmt = AL_SAMPFMT_TWOSCOMP;
                break;

            default:
                valid = 0;
                test_format = SDL_NextAudioFormat();
                break;
        }

        if (valid) {
            ALconfig audio_config = alNewConfig();
            valid = 0;
            if (audio_config) {
                if (alSetChannels(audio_config, _this->spec.channels) < 0) {
                    if (_this->spec.channels > 2) {
                        _this->spec.channels = 2;
                    }
                }

                if ((alSetSampFmt(audio_config, fmt) >= 0) &&
                    ((!width) || (alSetWidth(audio_config, width) >= 0)) &&
                    (alSetQueueSize(audio_config, _this->spec.samples * 2) >= 0) &&
                    (alSetChannels(audio_config, _this->spec.channels) >= 0)) {

                    _this->hidden->audio_port = alOpenPort("SDL audio", "w", audio_config);
                    if (_this->hidden->audio_port == NULL) {
                        int err = oserror();
                        if (err == AL_BAD_CHANNELS) {
                            _this->spec.channels = 2;
                            alSetChannels(audio_config, _this->spec.channels);
                            _this->hidden->audio_port = alOpenPort("SDL audio", "w",
                                                                    audio_config);
                        }
                    }

                    if (_this->hidden->audio_port != NULL) {
                        valid = 1;
                    }
                }

                alFreeConfig(audio_config);
            }
        }
    }

    if (!valid) {
        return SDL_SetError("Unsupported audio format");
    }

    /* Update the fragment size as size in bytes */
    SDL_CalculateAudioSpec(&_this->spec);

    /* Allocate mixing buffer */
    _this->hidden->mixbuf = (Uint8 *)SDL_malloc(_this->spec.size);
    if (!_this->hidden->mixbuf) {
        return SDL_OutOfMemory();
    }
    SDL_memset(_this->hidden->mixbuf, _this->spec.silence, _this->spec.size);

    return 0;
}

static SDL_bool IRIXAUDIO_Init(SDL_AudioDriverImpl *impl)
{
    impl->OpenDevice = IRIXAUDIO_OpenDevice;
    impl->WaitDevice = IRIXAUDIO_WaitDevice;
    impl->PlayDevice = IRIXAUDIO_PlayDevice;
    impl->GetDeviceBuf = IRIXAUDIO_GetDeviceBuf;
    impl->CloseDevice = IRIXAUDIO_CloseDevice;
    impl->OnlyHasDefaultOutputDevice = SDL_TRUE;

    return SDL_TRUE;
}

AudioBootStrap DMEDIA_bootstrap = {
    "dmedia", "IRIX DMedia audio", IRIXAUDIO_Init, SDL_FALSE
};

#endif /* SDL_AUDIO_DRIVER_DMEDIA */
