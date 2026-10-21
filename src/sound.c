#include "sound.h"

bool sound_init(sound_t *self, const char *file, float volume)
{
    if (!SDL_LoadWAV(file, &self->spec, &self->data, &self->data_len))
    {
        SDL_Log("Failed to load WAV for sound %s: %s", file, SDL_GetError());
        return false;
    }

    self->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &self->spec, NULL, NULL);
    if (self->stream == NULL)
    {
        SDL_Log("Failed to create audio stream for sound %s: %s", file, SDL_GetError());
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(self->stream))
    {
        SDL_Log("Failed to unpause audio stream for sound %s: %s", file, SDL_GetError());
        return false;
    }

    if (!SDL_SetAudioStreamGain(self->stream, volume))
    {
        SDL_Log("Failed to set gain for sound %s: %s", file, SDL_GetError());
        return false;
    }

    return true;
}

void sound_play(sound_t *self)
{
    if (SDL_GetAudioStreamQueued(self->stream) < (int)self->data_len)
    {
        SDL_PutAudioStreamData(self->stream, self->data, self->data_len);
        SDL_FlushAudioStream(self->stream);
    }
}

void sound_destroy(sound_t *self)
{
    SDL_DestroyAudioStream(self->stream);
    SDL_free(self->data);
}
