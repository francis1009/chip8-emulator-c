#include <stdbool.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "chip8.h"
#include "display.h"
#include "input.h"

int main(void) {
	bool is_running = display_init();
	if (!is_running) {
		fprintf(stderr, "Error: Failed to initialize display. Exiting.\n");
		return 1;
	}

	Chip8 chip8;
	chip8_init(&chip8);
	chip8_load_rom(&chip8, "roms/PONG");

	while (is_running) {
		process_input(&chip8, &is_running);

		chip8_emulate_cycle(&chip8);

		if (chip8.draw_flag) {
			display_draw(&chip8);
			// chip8.draw_flag = false;
		}

		SDL_Delay(100);
	}

	display_destroy();
	return 0;
}
