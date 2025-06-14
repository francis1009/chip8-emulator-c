#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>

typedef struct {
	// 2 byte current opcode
	unsigned short opcode;

	// 4K memory
	unsigned char memory[4096];

	// 15 General Purpose CPU registers V0-VE, 1 carry flag register
	unsigned char V[16];

	/*
	 * Index register and Program Counter (0x000 to 0xFFF)
	 * 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	 * 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	 * 0x200-0xFFF - Program ROM and work RAM
	 */
	unsigned short I;
	unsigned short pc;

	// Graphics system - XOR
	unsigned char gfx[64 * 32];
	bool draw_flag;

	// Timer registers
	unsigned char delay_timer;
	unsigned char sound_timer;

	// 16 levels of stack & stack pointer
	unsigned short stack[16];
	unsigned short sp;

	// HEX-based keypad (0x0 - 0xF)
	unsigned char key[16];
} Chip8;

extern const unsigned char chip8_fontset[80];

void chip8_init(Chip8 *chip8);
void chip8_load_rom(Chip8 *chip8, const char *filename);
void chip8_emulate_cycle(Chip8 *chip8);
void chip8_update_timers(Chip8 *chip8);

#endif
