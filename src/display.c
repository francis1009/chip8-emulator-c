#include "display.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "chip8.h"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32
#define WINDOW_SCALE 10

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static uint32_t pixel_buffer[64 * 32];

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
		SDL_DestroyWindow(window);
		SDL_Quit();
		window = NULL;
		return false;
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
															SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH,
															WINDOW_HEIGHT);
	if (texture == NULL) {
		fprintf(stderr, "Error creating texture: %s\n", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		renderer = NULL;
		window = NULL;
		return false;
	}

	if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)) {
		fprintf(stderr, "Nearest neighbour texture filtering failed to enable.");
	}

	printf("Display initialized successfully.\n");
	return true;
}

void display_draw(const Chip8 *chip8) {
	for (int i = 0; i < 2048; ++i) {
		// Summer Beach Day Palette
		if (chip8->gfx[i]) {
			// Foreground: Deep Sea Blue
			pixel_buffer[i] = 0xFF006994;
		} else {
			// Background: Sandy Beige
			pixel_buffer[i] = 0xFFF4E8D1;
		}
	}
	SDL_UpdateTexture(texture, NULL, pixel_buffer, 64 * sizeof(uint32_t));
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void display_destroy(void) {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	printf("Display destroyed.\n");

	texture = NULL;
	renderer = NULL;
	window = NULL;
}
