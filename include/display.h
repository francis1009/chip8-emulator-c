#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip8.h"

void display_init();
void display_draw(const Chip8 *chip8);
void display_destroy();

#endif
