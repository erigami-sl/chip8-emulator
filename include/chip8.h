#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define FONT_START 0x50
#define RAM_SIZE 4096
#define KEY_SIZE 16
#define REG_SIZE 16
//CHIP8 yapısının tanımı


uint8_t ram[RAM_SIZE];
uint8_t V[REG_SIZE];
uint16_t I;
uint8_t DT;
uint8_t ST;
uint16_t PC;
uint8_t SP;
uint16_t stack[16];
uint8_t key[KEY_SIZE];
uint8_t display[32][64]; //emin değilim!!!

//Sistemi çalışır kılmak için gereken fonksiyonların deklarasyonu

int init(chip8*);
int load_rom(chip8*, char*);
void dump(chip8*);
int cycle(chip8*);



#endif
