#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>

#include "audio.h"
#include "chip8.h"
#include "display.h"
#include "input.h"

const int TARGET_FPS = 60;
const float FRAME_DURATION_MS = 1000.0f / TARGET_FPS;
const int CYCLES_PER_FRAME = 8;

int main(int argc, char **argv) {
	srand(time(NULL));

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <rom_file_name>\n", argv[0]);
		return 1;
	}

	const char *rom_filename = argv[1];
	printf("Attempting to load ROM: %s\n", rom_filename);

	bool is_running = display_init();
	if (!is_running) {
		fprintf(stderr, "Error: Failed to initialize display. Exiting.\n");
		SDL_Quit();
		return 1;
	}

	bool audio_on = audio_init();
	if (!audio_on) {
		fprintf(stderr, "Error: Failed to initialize audio. Exiting.\n");
		display_destroy();
		SDL_Quit();
		return 1;
	}

	Chip8 chip8;
	chip8_init(&chip8);
	chip8_load_rom(&chip8, rom_filename);

	while (is_running) {
		Uint64 frame_start_time = SDL_GetTicks();

		memcpy(chip8.key_prev, chip8.key, sizeof(chip8.key));
		process_input(&chip8, &is_running);
		for (int i = 0; i < CYCLES_PER_FRAME; i++) {
			chip8_emulate_cycle(&chip8);
		}
		chip8_update_timers(&chip8);

		if (chip8.draw_flag) {
			display_draw(&chip8);
			chip8.draw_flag = false;
		}

		Uint64 frame_end_time = SDL_GetTicks();
		float elapsed_ms = (float) (frame_end_time - frame_start_time);
		if (elapsed_ms < FRAME_DURATION_MS) {
			SDL_Delay((Uint32) (FRAME_DURATION_MS - elapsed_ms));
		}
	}

	audio_destroy();
	display_destroy();
	SDL_Quit();
	return 0;
}
