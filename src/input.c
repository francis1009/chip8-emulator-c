#include "input.h"

#include <stdio.h>

#include <SDL3/SDL.h>

#include "chip8.h"

// A keymap to translate SDL scancodes to CHIP-8 key indexes.
// CHIP-8 Keypad:    SDL Keyboard:
// 1 2 3 C           1 2 3 4
// 4 5 6 D           Q W E R
// 7 8 9 E           A S D F
// A 0 B F           Z X C V
static const SDL_Scancode KEYMAP[16] = {
		SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
		SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
		SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
		SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V};

void process_input(Chip8 *chip8, bool *is_running) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		// Handle closing window
		case SDL_EVENT_QUIT:
			*is_running = false;
			return;

		// Handle key being pressed down
		case SDL_EVENT_KEY_DOWN:
			// Quit program if 'Esc' key is pressed
			if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
				*is_running = false;
				return;
			}

			for (int i = 0; i < 16; i++) {
				if (event.key.scancode == KEYMAP[i]) {
					chip8->key[i] = 1; // Set the key state to ON
				}
			}
			break;

		// Handle key being released
		case SDL_EVENT_KEY_UP:
			for (int i = 0; i < 16; i++) {
				if (event.key.scancode == KEYMAP[i]) {
					chip8->key[i] = 0; // Set the key state to OFF
				}
			}
			break;
		}
	}
}
