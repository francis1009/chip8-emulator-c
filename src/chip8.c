#include "chip8.h"

#include <stdio.h>
#include <string.h>

unsigned char chip8_fontset[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80	// F
};

void chip8_init(Chip8 *chip8) {
	chip8->pc = 0x200;
	chip8->opcode = 0;
	chip8->I = 0;
	chip8->sp = 0;

	memset(chip8->gfx, 0, sizeof(chip8->gfx));
	memset(chip8->stack, 0, sizeof(chip8->stack));
	memset(chip8->V, 0, sizeof(chip8->V));
	memset(chip8->memory, 0, sizeof(chip8->memory));

	for (int i = 0; i < 80; ++i) {
		chip8->memory[i] = chip8_fontset[i];
	}

	chip8->delay_timer = 0;
	chip8->sound_timer = 0;
}

void load_rom(Chip8 *char8, const char *filename) {
}

void emulate_cycle(Chip8 *chip8) {
	// Fetch opcode
	chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

	// Decode opcode
	switch (chip8->opcode & 0xF000) {
	case 0xA000: // ANNN: Sets I to the address NNN
		// Execute opcode
		chip8->I = chip8->opcode & 0x0FFF;
		chip8->pc += 2;
		break;

	default:
		printf("Unknown opcode: 0x%X\n", chip8->opcode);
	}

	// Update timers
	if (chip8->delay_timer > 0) {
		chip8->delay_timer--;
	}
	if (chip8->sound_timer > 0) {
		if (chip8->sound_timer == 1)
			printf("BEEP!\n");
		--chip8->sound_timer;
	}
}

void set_keys(Chip8 *chip8) {
}
