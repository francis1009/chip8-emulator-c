#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

#include "chip8.h"

bool display_init();
void display_draw(const Chip8 *chip8);
void display_destroy();

#endif
