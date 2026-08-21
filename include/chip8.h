#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define FONT_START 0x50
#define RAM_SIZE 4096
#define KEY_SIZE 16
#define REG_SIZE 16

//CHIP8 yapısının tanımı


extern uint8_t ram[RAM_SIZE];
extern uint8_t V[REG_SIZE];
extern uint16_t I;
extern uint8_t DT;
extern uint8_t ST;
extern uint16_t PC;
extern uint8_t SP;
extern uint16_t stack[16];
extern uint8_t key[KEY_SIZE];
extern uint8_t display[32][64];

//Sistemi çalışır kılmak için gereken fonksiyonların deklarasyonu

int init(void);
int load_rom(char*);
void dump(void);
int cycle(void);

// SDL ile ilgili fonksiyonlar.

bool init_sdl(void);
// Tüm SDL alt sistemlerini güvenli şekilde kapatır.
void cleanup_sdl(void);
// 64x32'lik bellek matrisini okuyarak ekrana çizer.
void draw_display(uint8_t display[32][64]);
// Olayları (klavye ve pencere) yakalar.
// quit_flag: Pencere kapatılırsa true yapılır.
// keypad: 16 elemanlı tuş durumu dizisi (Basılıysa 1, değilse 0).
void handle_input(bool *quit_flag, uint8_t keypad[16]);
// Sesi açar (is_playing = true) veya kapatır (is_playing = false).
void set_sound(bool is_playing);



#endif
