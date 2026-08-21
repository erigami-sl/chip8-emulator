#include "chip8.h"
uint8_t ram[RAM_SIZE];
uint8_t V[REG_SIZE];
uint16_t I;
uint8_t DT;
uint8_t ST;
uint16_t PC;
uint8_t SP;
uint16_t stack[16];
uint8_t key[KEY_SIZE];
uint8_t display[32][64];
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define DELAY_TIME 100
#include <SDL2/SDL.h>
#include <stdint.h>

#define SCREEN_WIDTH 640  // Pencere genişliği (Ölçeklenmiş)
#define SCREEN_HEIGHT 320 // Pencere yüksekliği (Ölçeklenmiş)
#define EMU_WIDTH 64      // Emülatör iç çözünürlüğü
#define EMU_HEIGHT 32     // Emülatör iç çözünürlüğü

static const uint8_t font[80]={
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
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// --- SDL NESNELERİ VE CALLBACK FONKSİYONLARI ---
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static SDL_AudioDeviceID audio_device;

// Ses için 440Hz kare dalga üreten callback fonksiyonu
static void audio_callback(void* userdata, Uint8* stream, int len) {
    static uint32_t audio_phase = 0;
    int16_t* buffer = (int16_t*)stream;
    int length = len / 2; // 16-bit format kullanıldığı için uzunluk yarıya düşer
    int sample_rate = 44100;
    int frequency = 440;
    int half_period = sample_rate / frequency / 2;

    for (int i = 0; i < length; i++) {
        // Periyodun yarısında pozitif (+3000), diğer yarısında negatif (-3000) genlik
        buffer[i] = ((audio_phase / half_period) % 2) ? 3000 : -3000;
        audio_phase++;
    }
}

int init(void) {
    memset(ram, 0, sizeof(ram));
    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));
    memset(key, 0, sizeof(key));
    memset(display, 0, sizeof(display));
    
    I = 0;
    DT = 0;
    ST = 0;
    SP = 0;

    PC = 0x200;
    memcpy(ram + FONT_START, font, sizeof(font));
    
    srand(time(NULL));
    
    return 0;
}

int load_rom(char *rom_file_addr){
	FILE *fp=fopen(rom_file_addr, "rb");
	if (fp == NULL) {
		perror("ROM dosyasi acilamadi");
		return -1;
	}
	fseek(fp, 0 , SEEK_END);
	long file_size=ftell(fp);
	rewind(fp);

	if (file_size>0xFFF-0x1FF){
		fclose(fp);
		return -1;
	}
	
	size_t bytes_read = fread(ram + 0x200, 1, file_size, fp);
	fclose(fp);
	
	if ((long)bytes_read != file_size) {
		return -1;
	}
	return 0;
}

void dump(void){

	//Debug için kullanılmak üzere istenen çıktılar yazılıyor.
	printf("PC = 0x%04X	SP = 0x%02X	I = 0x%04X\n", PC, SP, I);

	for(int j=0; j<2;j++){
		for (int i=0; i<8;i++){
			printf("V%01X=%02X ",i+j*8, V[i+j*8]);
		}
		printf("\n");
	}

	printf("DT=%02X	ST=%02X\n",DT, ST);
	
	//STACK EKLENECEK!
}

int cycle(void){
	//opcode 2 byte RAM verisi okuyarak elde edildi (16 bit uzunluğunda)
	//FETCH
	uint16_t op = (ram[PC] << 8) | ram[PC + 1];
	PC += 2;

	// PC 4096'yı aştıysa otomatik olarak 0'a döndür (Wrap-around)
    PC &= 0x0FFF;

	//DECODE
	uint8_t  X   = (op & 0x0F00) >> 8;
	uint8_t  Y   = (op & 0x00F0) >> 4;
	uint8_t  N   = (op & 0x000F);
	uint8_t  NN  = (op & 0x00FF);
	uint16_t NNN = (op & 0x0FFF);

	//EXECUTE
	switch(op & 0xF000) {
	    case 0x0000:
	        switch (NN) {
	            case 0xE0: //CLS: Clear the display
					memset(display, 0, sizeof(display));
	            	break;
	            case 0xEE://RET: Return from a subroutine.
					if(SP==0){
						fprintf(stderr, "Fatal Error: Stack Underflow at PC = 0x%04X\n", PC);
						return -1;
					}
	            	SP--;
	            	PC=stack[SP];
	            	break;
	            default:
	            	break;
	        }
	        break;

	    case 0x1000: //JP addr: Jump to location nnn.
			PC=NNN;
	    	break;

	    case 0x2000: //CALL addr: Call subroutine at nnn.
			if(SP>=15){
				fprintf(stderr, "Fatal Error: Stack Overflow at PC = 0x%04X\n", PC);
				return -1;
			}
			stack[SP]=PC;
			SP++;
			PC=NNN;
	    	break;

	    case 0x3000: //SE Vx, byte: Skip next instruction if Vx = kk.
			if(NN == V[X]){
				PC +=2;
			}
	    	break;

		case 0x4000: //4xkk - SNE Vx, byte: Skip next instruction if Vx != kk.
			if(NN != V[X]){
				PC += 2;
			}
			break;

		case 0x5000: //5xy0 - SE Vx, Vy: Skip next instruction if Vx = Vy.
			if(V[X]==V[Y]){
				PC += 2;
			}
			break;

		case 0x6000: // 6xkk - LD Vx, byte: Set Vx = kk.
			V[X]=NN;
			break;

		case 0x7000: //7xkk - ADD Vx, byte:Set Vx = Vx + kk.
			V[X]=V[X]+NN;
			break;

	    case 0x8000:
	        switch (N) {
				case 0x0: //8xy0 - LD Vx, Vy: Set Vx = Vy.
					V[X]=V[Y];
					break;

				case 0x1: //8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy.
					V[X]=V[X] | V[Y];
					break;

				case 0x2: //8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy.
					V[X]=V[X] & V[Y];
					break;

				case 0x3: //8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy.
					V[X]=V[X] ^ V[Y];
					break;

				case 0x4: { // 8xy4 - ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry.
					uint16_t sum = V[X] + V[Y];
					uint8_t carry = (sum > 0xFF) ? 1 : 0;
					V[X] = sum & 0xFF;
					V[0xF] = carry;
					break;
				}

				case 0x5: { // 8xy5 - SUB Vx, Vy:  Set Vx = Vx - Vy, set VF = NOT borrow.
					uint8_t notborrow = (V[X] >= V[Y]) ? 1 : 0;
					uint8_t diff = V[X] - V[Y];
					V[X] = diff;
					V[0xF] = notborrow;
					break;
				}

				case 0x6: { //8xy6 - SHR Vx {, Vy}: Set Vx = Vx SHR 1.
					uint8_t odd = ((V[X] & 0x01) == 1) ? 1 : 0;
					uint8_t div = V[X]/2;
					V[X] = div;
					V[0xF] = odd;
					break;
				}

				case 0x7: { //8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow.
					uint8_t notborrow = (V[Y] >= V[X]) ? 1 : 0;
					uint8_t diff = V[Y] - V[X];
					V[X] = diff;
					V[0xF] = notborrow;
					break;
				}

				case 0xE: { //8xyE - SHL Vx {, Vy}: Set Vx = Vx SHL 1.
					uint8_t flag = ((V[X] & 0x80) != 0) ? 1 : 0;
					uint8_t mult = V[X]*2;
					V[X] = mult;
					V[0xF] = flag;
					break;
				}

				default:
					break;
	        }
	        break;

	    case 0x9000: //9xy0 - SNE Vx, Vy: Skip next instruction if Vx != Vy.
			if(V[X] != V[Y]){
				PC += 2;
			}
			break;

		case 0xA000: //Annn - LD I, addr: Set I = nnn.
			I = NNN;
			break;

		case 0xB000: //Bnnn - JP V0, addr: Jump to location nnn + V0.
			PC = NNN + V[0];
			break;

		case 0xC000: //Cxkk - RND Vx, byte: Set Vx = random byte AND kk.
			V[X] = rand() & NN;
			break;

		case 0xD000: { //Dxyn - DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
			if(I>0xFFF){
				fprintf(stderr, "Fatal Error: I>RAM = 0x%04X\n", PC);
			}
			else{
				V[0xF]=0;
				for (int row=0; row<N; row++){

					uint8_t sprite = ram[I+row];

					for (int col=0; col<8; col++){
						int px = (V[X] + col) % 64;
						int py = (V[Y] + row) % 32;

						if (sprite & (0x80 >> col)){
							if (display[py][px]) {
								V[0xF] = 1;
							}
							display[py][px] ^= 1;
						}
					}
				}
			}
			break;
		}

	    case 0xE000:
	        switch (NN) {
				case 0x9E: //Ex9E - SKP Vx: Skip next instruction if key with the value of Vx is pressed.
					if (key[V[X]] == 1){
						PC+=2;
					}
					break;
				case 0xA1: //ExA1 - SKNP Vx: Skip next instruction if key with the value of Vx is not pressed.
					if (key[V[X]] == 0){
						PC+=2;
					}
					break;
	            default:
					break;
	        }
	        break;

	    case 0xF000:
	        switch (NN) {
				case 0x07: //Fx07 - LD Vx, DT: Set Vx = delay timer value.
					V[X] = DT;
					break;

				case 0x0A: { //Fx0A - LD Vx, K: Wait for a key press, store the value of the key in Vx.
					uint8_t key_pressed = 0;
					for (int i = 0; i < 16; i++) {
						if (key[i]) {
							V[X] = i;
							key_pressed = 1;
							break;
						}
					}
					if (!key_pressed) {
						PC -= 2; // Tekrar bu instruction'ı çalıştır
					}
					break;
				}

				case 0x15: //Fx15 - LD DT, Vx: Set delay timer = Vx.
					DT = V[X];
					break;

				case 0x18: //Fx18 - LD ST, Vx: Set sound timer = Vx.
					ST=V[X];
					break;

				case 0x1E: //Fx1E - ADD I, Vx: Set I = I + Vx.
					I = I + V[X];
					break;

				case 0x29: //Fx29 - LD F, Vx: Set I = location of sprite for digit Vx.
					I = FONT_START + 5*V[X];
					break;

				case 0x33: { //Fx33 - LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2.
					uint8_t bcd = V[X];
					ram[I] = bcd / 100;
					ram[I+1] = (bcd % 100) / 10;
					ram[I+2] = (bcd % 10);
					break;
				}

				case 0x55: { //Fx55 - LD [I], Vx: Store registers V0 through Vx in memory starting at location I.
					for (int i=0; i <= X; i++){
						ram[I+i] = V[i];
					}
					break;
				}

	            case 0x65: { //Fx65 - LD Vx, [I]: Read registers V0 through Vx from memory starting at location I.
					for (int i=0; i <= X; i++){
						V[i] = ram[I + i];
					}
					break;
				}

	            default:
					break;
	        }
	        break;

	    default:
			break;
	}

return 0;
}


// --- SDL FONKSİYON IMPLEMENTASYONLARI ---

bool init_sdl(void) {
    // Hem Video hem de Audio alt sistemlerini başlat
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL Init Error: %s\n", SDL_GetError());
        return false;
    }

    // Pencere oluştur
    window = SDL_CreateWindow("CHIP-8 Emulator",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;

    // GPU hızlandırmalı renderer oluştur
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    // Streaming erişimli texture (ekran belleği) oluştur
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                EMU_WIDTH, EMU_HEIGHT);
    if (!texture) return false;

    // Ses cihazını yapılandır ve aç
    SDL_AudioSpec want, have;
    memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_device == 0) {
        fprintf(stderr, "Audio Init Error: %s\n", SDL_GetError());
        // Ses aygıtı açılamazsa uygulamayı çökertmemek için false dönmüyoruz
    }

    return true;
}

void cleanup_sdl(void) {
    if (audio_device > 0) SDL_CloseAudioDevice(audio_device);
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void draw_display(uint8_t display_matrix[32][64]) {
    uint32_t pixel_buffer[EMU_WIDTH * EMU_HEIGHT];

    // 2D 8-bit matrisi, 1D 32-bit (RGBA) diziye dönüştür
    for (int y = 0; y < EMU_HEIGHT; y++) {
        for (int x = 0; x < EMU_WIDTH; x++) {
            if (display_matrix[y][x] != 0) {
                pixel_buffer[(y * EMU_WIDTH) + x] = 0xFFFFFFFF; // Beyaz
            } else {
                pixel_buffer[(y * EMU_WIDTH) + x] = 0x000000FF; // Siyah
            }
        }
    }

    // Texture'ı güncelle ve ekrana bas
    SDL_UpdateTexture(texture, NULL, pixel_buffer, EMU_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void handle_input(bool *quit_flag, uint8_t keypad[16]) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *quit_flag = true;
        }

        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            uint8_t state = (event.type == SDL_KEYDOWN) ? 1 : 0;

            // CHIP-8 (16'lık) Hex Tuş Takımının Modern Klavyeye Eşlenmesi
            switch (event.key.keysym.sym) {
                case SDLK_x: keypad[0] = state; break;
                case SDLK_1: keypad[1] = state; break;
                case SDLK_2: keypad[2] = state; break;
                case SDLK_3: keypad[3] = state; break;
                case SDLK_q: keypad[4] = state; break;
                case SDLK_w: keypad[5] = state; break;
                case SDLK_e: keypad[6] = state; break;
                case SDLK_a: keypad[7] = state; break;
                case SDLK_s: keypad[8] = state; break;
                case SDLK_d: keypad[9] = state; break;
                case SDLK_z: keypad[10] = state; break;
                case SDLK_c: keypad[11] = state; break;
                case SDLK_4: keypad[12] = state; break;
                case SDLK_r: keypad[13] = state; break;
                case SDLK_f: keypad[14] = state; break;
                case SDLK_v: keypad[15] = state; break;
            }
        }
    }
}

void set_sound(bool is_playing) {
    if (audio_device > 0) {
        // 0 = Çalıştır, 1 = Duraklat (Pause)
        SDL_PauseAudioDevice(audio_device, is_playing ? 0 : 1);
    }
}

