#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

//CHIP8 yapısının tanımı

typedef struct {
	uint8_t ram[4096];
	uint8_t V[16];
	uint16_t I;
	uint8_t DT;
	uint8_t ST;
	uint16_t PC;
	uint8_t SP;
	uint16_t stack[16];
	uint16_t keyboard;
	uint64_t display[32]; //emin değilim!!!
} chip8;

//Sistemi çalışır kılmak için gereken fonksiyonların deklarasyonu

int init(chip8*);
int load_rom(chip8*, char*);
void dump(chip8*);
int cycle(chip8*);

#endif
