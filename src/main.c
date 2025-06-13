#include <stdbool.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "chip8.h"
#include "display.h"
#include "input.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <rom_file_name>\n", argv[0]);
		return 1;
	}

	const char *rom_filename = argv[1];
	printf("Attempting to load ROM: %s\n", rom_filename);

	bool is_running = display_init();
	if (!is_running) {
		fprintf(stderr, "Error: Failed to initialize display. Exiting.\n");
		return 1;
	}

	Chip8 chip8;
	chip8_init(&chip8);
	chip8_load_rom(&chip8, rom_filename);

	while (is_running) {
		process_input(&chip8, &is_running);

		chip8_emulate_cycle(&chip8);

		if (chip8.draw_flag) {
			display_draw(&chip8);
			chip8.draw_flag = false;
		}

		SDL_Delay(100);
	}

	display_destroy();
	return 0;
}
