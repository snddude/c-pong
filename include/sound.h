#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

typedef struct
{
    float volume;

    uint8_t *data;
    uint32_t data_len;

    SDL_AudioSpec spec;
    SDL_AudioStream *stream;
} sound_t;

bool sound_init(sound_t *self, const char *file, float volume);
void sound_play(sound_t *self);
void sound_destroy(sound_t *self);
