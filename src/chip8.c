#include "chip8.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"

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
	memset(chip8->key, 0, sizeof(chip8->key));
	memset(chip8->key_prev, 0, sizeof(chip8->key_prev));

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
	case 0x0000:
		switch (chip8->opcode) {
		case 0x00E0: // 00E0: Clears the screen
			// Execute opcode
			memset(chip8->gfx, 0, sizeof(chip8->gfx));
			chip8->draw_flag = true;
			chip8->pc += 2;
			break;

		case 0x00EE: // 00EE: Returns from subroutine
			chip8->sp--;
			chip8->pc = chip8->stack[chip8->sp];
			chip8->pc += 2;
			break;

		default: // 0NNN: Execute machine language subroutine at address NNN
			printf("Ignoring SYS opcode: 0x%X\n", chip8->opcode); // Skip opcode
			chip8->pc += 2;
		}
		break;

	case 0x1000: // 1NNN: Jump to address NNN
		chip8->pc = chip8->opcode & 0x0FFF;
		break;

	case 0x2000: // 2NNN: Execute subroutine starting at address NNN
		chip8->stack[chip8->sp] = chip8->pc;
		chip8->sp++;
		chip8->pc = chip8->opcode & 0x0FFF;
		break;

	case 0x3000: // 3XNN: Skip the following instruction if the value of register
							 // VX equals NN
		if (chip8->V[(chip8->opcode & 0x0F00) >> 8] == (chip8->opcode & 0x00FF)) {
			chip8->pc += 2;
		}
		chip8->pc += 2;
		break;

	case 0x4000: // 4XNN: Skip the following instruction if the value of register
		// VX is not equal to NN
		if (chip8->V[(chip8->opcode & 0x0F00) >> 8] != (chip8->opcode & 0x00FF)) {
			chip8->pc += 2;
		}
		chip8->pc += 2;
		break;

	case 0x5000: // 5XY0: Skip the following instruction if the value of register
							 // VX is equal to the value of register VY
		if (chip8->V[(chip8->opcode & 0x0F00) >> 8] ==
				chip8->V[(chip8->opcode & 0x00F0) >> 4]) {
			chip8->pc += 2;
		}
		chip8->pc += 2;
		break;

	case 0x6000: // 6XNN: Store number NN in register VX
		chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->opcode & 0x00FF;
		chip8->pc += 2;
		break;

	case 0x7000: // 7XNN: Add the value NN to register VX
		chip8->V[(chip8->opcode & 0x0F00) >> 8] += chip8->opcode & 0x00FF;
		chip8->pc += 2;
		break;

	case 0x8000:
		switch (chip8->opcode & 0x000F) {
		case 0x0000: // 8XY0: Store the value of register VY in register VX
			chip8->V[(chip8->opcode & 0x0F00) >> 8] =
					chip8->V[(chip8->opcode & 0x00F0) >> 4];
			chip8->pc += 2;
			break;

		case 0x0001: // 8XY1: Set VX to VX OR VY
			chip8->V[(chip8->opcode & 0x0F00) >> 8] |=
					chip8->V[(chip8->opcode & 0x00F0) >> 4];
			chip8->V[0xF] = 0;
			chip8->pc += 2;
			break;

		case 0x0002: // 8XY2: Set VX to VX AND VY
			chip8->V[(chip8->opcode & 0x0F00) >> 8] &=
					chip8->V[(chip8->opcode & 0x00F0) >> 4];
			chip8->V[0xF] = 0;
			chip8->pc += 2;
			break;

		case 0x0003: // 8XY3: Set VX to VX XOR VY
			chip8->V[(chip8->opcode & 0x0F00) >> 8] ^=
					chip8->V[(chip8->opcode & 0x00F0) >> 4];
			chip8->V[0xF] = 0;
			chip8->pc += 2;
			break;

		case 0x0004: // 8XY4: Add the value of register VY to register VX
								 // Set VF to 01 if a carry occurs
								 // Set VF to 00 if a carry does not occur
		{
			unsigned char vx = (chip8->opcode & 0x0F00) >> 8;
			unsigned char vy = (chip8->opcode & 0x00F0) >> 4;
			unsigned short sum = chip8->V[vx] + chip8->V[vy];

			chip8->V[vx] = sum & 0xFF;
			chip8->V[0xF] = 0;
			if (sum > 255) {
				chip8->V[0xF] = 1;
			}
			chip8->pc += 2;
			break;
		}

		case 0x0005: // 8XY5: Subtract the value of register VY from register VX
								 // Set VF to 00 if a borrow occurs
								 // Set VF to 01 if a borrow does not occur
		{
			unsigned char vx = (chip8->opcode & 0x0F00) >> 8;
			unsigned char vy = (chip8->opcode & 0x00F0) >> 4;
			signed short diff = chip8->V[vx] - chip8->V[vy];

			chip8->V[vx] = diff & 0xFF;
			chip8->V[0xF] = 1;
			if (diff < 0) {
				chip8->V[0xF] = 0;
			}
			chip8->pc += 2;
			break;
		}

		case 0x0006: // 8XY6: Store the value of register VY shifted right one bit
								 // in register VX
								 // Set register VF to the least significant bit prior to the
								 // shift
								 // VY is unchanged
		{
			chip8->V[(chip8->opcode & 0x0F00) >> 8] =
					chip8->V[(chip8->opcode & 0x00F0) >> 4];
			unsigned char lsb = chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x01;
			chip8->V[(chip8->opcode & 0x0F00) >> 8] >>= 1;
			chip8->V[0xF] = lsb;
			chip8->pc += 2;
			break;
		}

		case 0x0007: // 8XY7: Set register VX to the value of VY minus VX
								 // Set VF to 00 if a borrow occurs
								 // Set VF to 01 if a borrow does not occur
		{
			unsigned char vx = (chip8->opcode & 0x0F00) >> 8;
			unsigned char vy = (chip8->opcode & 0x00F0) >> 4;
			signed short diff = chip8->V[vy] - chip8->V[vx];

			chip8->V[vx] = diff & 0xFF;
			chip8->V[0xF] = 1;
			if (diff < 0) {
				chip8->V[0xF] = 0;
			}
			chip8->pc += 2;
			break;
		}

		case 0x000E: // 8XYE: Store the value of register VY shifted left one bit
								 // in register VX
								 // Set register VF to the most significant bit prior to the
								 // shift
								 // VY is unchanged
		{
			chip8->V[(chip8->opcode & 0x0F00) >> 8] =
					chip8->V[(chip8->opcode & 0x00F0) >> 4];
			unsigned char msb = (chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x80) >> 7;
			chip8->V[(chip8->opcode & 0x0F00) >> 8] <<= 1;
			chip8->V[0xF] = msb;
			chip8->pc += 2;
			break;
		}

		default:
			printf("Unknown opcode [0x8000]: 0x%X\n", chip8->opcode);
		}
		break;

	case 0x9000: // 9XY0: Skip the following instruction if the value of register
							 // VX is not equal to the value of register VY
		if (chip8->V[(chip8->opcode & 0x0F00) >> 8] !=
				chip8->V[(chip8->opcode & 0x00F0) >> 4]) {
			chip8->pc += 2;
		}
		chip8->pc += 2;
		break;

	case 0xA000: // ANNN: Store memory address NNN in register I
		chip8->I = chip8->opcode & 0x0FFF;
		chip8->pc += 2;
		break;

	case 0xB000: // BNNN: Jump to address NNN + V0
		chip8->pc = (chip8->opcode & 0x0FFF) + chip8->V[0x0];
		break;

	case 0xC000: // CXNN: Set VX to a random number with a mask of NN
	{
		unsigned char random_number = rand() % 256;
		chip8->V[(chip8->opcode & 0x0F00) >> 8] =
				random_number & (chip8->opcode & 0x00FF);
		chip8->pc += 2;
		break;
	}

	case 0xD000: // DXYN: Draw a sprite at position VX, VY with N bytes of
							 // sprite data starting at the address stored in I
							 // Set VF to 01 if any set pixels are changed to unset, and 00
							 // otherwise
	{
		unsigned short cx = chip8->V[(chip8->opcode & 0x0F00) >> 8];
		unsigned short cy = chip8->V[(chip8->opcode & 0x00F0) >> 4];
		unsigned short height = chip8->opcode & 0x000F;
		unsigned short pixel;

		chip8->V[0xF] = 0;

		// Clip if partial sprite out of screen
		if (cx < 64 && cy < 32) {
			for (int y = 0; y < height; y++) {
				pixel = chip8->memory[chip8->I + y];
				if (cy + y >= 32) {
					continue;
				}
				for (int x = 0; x < 8; x++) {
					if (cx + x >= 64) {
						continue;
					}
					if ((pixel & (0x80 >> x)) != 0) {
						if (chip8->gfx[(cx + x) + (cy + y) * 64] == 1) {
							chip8->V[0xF] = 1;
						}
						chip8->gfx[(cx + x) + (cy + y) * 64] ^= 1;
					}
				}
			}

			// Wrap if whole sprite out of screen
		} else {
			for (int y = 0; y < height; y++) {
				pixel = chip8->memory[chip8->I + y];
				for (int x = 0; x < 8; x++) {
					if ((pixel & (0x80 >> x)) != 0) {
						if (chip8->gfx[(cx + x) % 64 + ((cy + y) % 32) * 64] == 1) {
							chip8->V[0xF] = 1;
						}
						chip8->gfx[(cx + x) % 64 + ((cy + y) % 32) * 64] ^= 1;
					}
				}
			}
		}

		chip8->draw_flag = true;
		chip8->pc += 2;
		break;
	}

	case 0xE000:
		switch (chip8->opcode & 0x00FF) {
		case 0x009E: // EX9E: Skip the following instruction if the key
								 // corresponding to the hex value currently stored in register
								 // VX is pressed
		{
			unsigned char key = chip8->V[(chip8->opcode & 0x0F00) >> 8];
			if (chip8->key[key] == 1) {
				chip8->pc += 2;
			}
			chip8->pc += 2;
			break;
		}

		case 0x00A1: // EXA1: Skip the following instruction if the key
								 // corresponding to the hex value currently stored in register
								 // VX is not pressed
		{
			unsigned char key = chip8->V[(chip8->opcode & 0x0F00) >> 8];
			if (chip8->key[key] == 0) {
				chip8->pc += 2;
			}
			chip8->pc += 2;
			break;
		}

		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", chip8->opcode);
		}
		break;

	case 0xF000:
		switch (chip8->opcode & 0x00FF) {
		case 0x0007: // FX07: Store the current value of the delay timer in register
								 // VX
			chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->delay_timer;
			chip8->pc += 2;
			break;

		case 0x000A: // FX0A: Wait for a keypress and store the result in register
								 // VX
		{
			for (int i = 0; i < 16; i++) {
				if (chip8->key[i] == 0 && chip8->key_prev[i] == 1) {
					chip8->V[(chip8->opcode & 0x0F00) >> 8] = i;
					chip8->pc += 2;
					break;
				}
			}
			break;
		}

		case 0x0015: // FX15: Set the delay timer to the value of register VX
			chip8->delay_timer = chip8->V[(chip8->opcode & 0x0F00) >> 8];
			chip8->pc += 2;
			break;

		case 0x0018: // FX18: Set the sound timer to the value of register VX
			chip8->sound_timer = chip8->V[(chip8->opcode & 0x0F00) >> 8];
			chip8->pc += 2;
			break;

		case 0x001E: // FX1E: Add the value stored in register VX to register I
			chip8->I += chip8->V[(chip8->opcode & 0x0F00) >> 8];
			chip8->pc += 2;
			break;

		case 0x0029: // FX29: Set I to the memory address of the sprite data
								 // corresponding to the hexadecimal digit stored in register VX
		{
			unsigned char font = chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x0F;
			chip8->I = font * 5;
			chip8->pc += 2;
			break;
		}

		case 0x0033: // FX33: Store the binary-coded decimal equivalent of the value
								 // stored in register VX at addresses I, I + 1, and I + 2
		{
			unsigned char value = chip8->V[(chip8->opcode & 0x0F00) >> 8];
			for (int i = 2; i >= 0; i--) {
				chip8->memory[chip8->I + i] = value % 10;
				value /= 10;
			}
			chip8->pc += 2;
			break;
		}

		case 0x0055: // FX55: Store the values of registers V0 to VX inclusive in
								 // memory starting at address I I is set to I + X + 1 after
								 // operation
		{
			unsigned char num_registers = (chip8->opcode & 0x0F00) >> 8;
			for (int i = 0; i <= num_registers; i++) {
				chip8->memory[chip8->I + i] = chip8->V[i];
			}
			chip8->I += num_registers + 1;
			chip8->pc += 2;
			break;
		}

		case 0x0065: // FX65: Fill registers V0 to VX inclusive with the values
								 // stored in memory starting at address I
								 // I is set to I + X + 1 after operation
		{
			unsigned char num_registers = (chip8->opcode & 0x0F00) >> 8;
			for (int i = 0; i <= num_registers; i++) {
				chip8->V[i] = chip8->memory[chip8->I + i];
			}
			chip8->I += num_registers + 1;
			chip8->pc += 2;
			break;
		}

		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", chip8->opcode);
		}
		break;

	default:
		printf("Unknown opcode: 0x%X\n", chip8->opcode);
	}
}

void chip8_update_timers(Chip8 *chip8) {
	// Update timers
	if (chip8->delay_timer > 0) {
		chip8->delay_timer--;
	}
	if (chip8->sound_timer > 0) {
		audio_beep_on();
		chip8->sound_timer--;
	} else {
		audio_beep_off();
	}
}
