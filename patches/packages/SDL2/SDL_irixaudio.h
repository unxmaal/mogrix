/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  IRIX dmedia audio backend, adapted from sgug-rse SDL2 patch.
  Updated for SDL2 2.30.x API.
*/
#ifndef SDL_irixaudio_h_
#define SDL_irixaudio_h_

#include <dmedia/audio.h>

#include "../SDL_sysaudio.h"

/* Re-define _THIS after SDL_sysaudio.h undefs it */
#define _THIS SDL_AudioDevice *_this

struct SDL_PrivateAudioData {
    ALport audio_port;
    Uint8 *mixbuf;
};

#endif /* SDL_irixaudio_h_ */
