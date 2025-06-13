#include "chip8.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

const unsigned char chip8_fontset[80] = {
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

	chip8->draw_flag = 1;
}

void chip8_load_rom(Chip8 *chip8, const char *filename) {
	char full_path[256];
	int result = snprintf(full_path, sizeof(full_path), "roms/%s", filename);
	if (result < 0 || result >= (int) sizeof(full_path)) {
		fprintf(stderr, "Error: ROM filename is too long.\n");
		return;
	}

	FILE *file = fopen(full_path, "rb");
	if (file == NULL) {
		perror("Error opening ROM file\n");
		return;
	}

	fseek(file, 0, SEEK_END);
	long rom_size = ftell(file);
	rewind(file);
	if (rom_size > 4096 - 512) {
		fprintf(stderr, "Error: ROM file size (%ld bytes) is too large.\n",
						rom_size);
		fclose(file);
		return;
	}

	size_t bytes_read = fread(&chip8->memory[512], 1, rom_size, file);
	if (bytes_read != (size_t) rom_size) {
		fprintf(stderr, "Error while reading ROM file.\n");
		fclose(file);
		return;
	}

	fclose(file);
	printf("Successfully loaded ROM: %s (%ld bytes)\n", filename, rom_size);
}

void chip8_emulate_cycle(Chip8 *chip8) {
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
