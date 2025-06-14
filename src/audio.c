#include "audio.h"

#include <stdbool.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_stdinc.h>

const int AMPLITUDE = 10000;
const int TONE_HZ = 440;
const int SAMPLE_RATE = 44100;

static SDL_AudioDeviceID device_id;
static SDL_AudioStream *stream = NULL;

static volatile bool is_beeping = false;

static void audio_callback(void *userdata, SDL_AudioStream *stream,
													 int additional_amount, int total_amount) {
	(void) userdata;
	(void) additional_amount;

	int num_samples = total_amount / sizeof(Sint16);
	Sint16 sample_buffer[num_samples];

	static const int wave_period = SAMPLE_RATE / TONE_HZ;
	static unsigned int wave_pos = 0;

	for (int i = 0; i < num_samples; i++) {
		if (is_beeping) {
			if ((wave_pos / (wave_period / 2)) % 2) {
				sample_buffer[i] = AMPLITUDE;
			} else {
				sample_buffer[i] = -AMPLITUDE;
			}
		} else {
			sample_buffer[i] = 0;
		}
		wave_pos++;
	}

	SDL_PutAudioStreamData(stream, sample_buffer, total_amount);
}

bool audio_init(void) {
	SDL_AudioSpec desired_spec = {
			.format = SDL_AUDIO_S16, .channels = 1, .freq = SAMPLE_RATE};

	device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (device_id == 0) {
		fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
		return false;
	}

	stream = SDL_CreateAudioStream(&desired_spec, NULL);
	if (stream == NULL) {
		fprintf(stderr, "Error creating audio stream: %s", SDL_GetError());
		return false;
	}

	if (SDL_BindAudioStream(device_id, stream) == false) {
		fprintf(stderr, "Failed to bind audio stream: %s", SDL_GetError());
		return false;
	}

	SDL_SetAudioStreamGetCallback(stream, audio_callback, NULL);
	SDL_ResumeAudioDevice(device_id);

	return true;
}

void audio_beep_on(void) {
	is_beeping = true;
}

void audio_beep_off(void) {
	is_beeping = false;
}

void audio_destroy(void) {
	SDL_PauseAudioDevice(device_id);
	SDL_DestroyAudioStream(stream);
	SDL_CloseAudioDevice(device_id);
}
