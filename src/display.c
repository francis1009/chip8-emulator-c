#include "display.h"

#include <stdbool.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "chip8.h"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32
#define WINDOW_SCALE 10

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

bool display_init(void) {
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
		return false;
	}

	window =
			SDL_CreateWindow("Chip-8 Emulator Window", WINDOW_WIDTH * WINDOW_SCALE,
											 WINDOW_HEIGHT * WINDOW_SCALE, 0);
	if (window == NULL) {
		fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	renderer = SDL_CreateRenderer(window, NULL);
	if (renderer == NULL) {
		fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	printf("Display initialized successfully.\n");
	return true;
}

void display_draw(const Chip8 *chip8) {
	SDL_SetRenderDrawColor(renderer, 10, 10, 40, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

void display_destroy(void) {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_Quit();
	printf("Display destroyed.\n");
}
