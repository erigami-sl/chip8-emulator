#include "chip8.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

int init(void){
	//CHIP 0'a initialize ediliyor.
	memset(chip, 0, sizeof(chip8));
	//RAM ve PC kuruluyor.
	memcpy(ram+0x050, font, sizeof(font));
	PC=0x200;

	srand(time(NULL));
	return 0;
}

int load_rom(char *rom_file_addr){
	FILE *fp=fopen(rom_file_addr, "rb");
	fseek(fp, 0 , SEEK_END);
	long file_size=ftell(fp);
	rewind(fp);

	if (file_size>0xFFF-0x1FF){
		return -1;
	}
	else{
		memcpy(ram+0x200, fp, file_size);
		return 0;
	}
	
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
	uint16_t op = (ram[c->PC] << 8) | ram[c->PC + 1];
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
					PC += 2;
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
			PC += 2;
	    	break;

		case 0x4000: //4xkk - SNE Vx, byte: Skip next instruction if Vx != kk.
			if(NN != V[X]){
				PC += 2;
			}
			PC += 2;
			break;

		case 0x5000: //5xy0 - SE Vx, Vy: Skip next instruction if Vx = Vy.
			if(V[X]==V[Y]){
				PC += 2;
			}
			PC += 2;
			break;

		case 0x6000: // 6xkk - LD Vx, byte: Set Vx = kk.
			V[X]=NN;
			PC += 2;
			break;

		case 0x7000: //7xkk - ADD Vx, byte:Set Vx = Vx + kk.
			V[X]=V[X]+NN;
			PC += 2;
			break;

	    case 0x8000:
	        switch (N) {
				case 0x0: //8xy0 - LD Vx, Vy: Set Vx = Vy.
					V[X]=V[Y];
					PC += 2;
					break;

				case 0x1: //8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy.
					V[X]=V[X] | V[Y];
					PC += 2;
					break;

				case 0x2: //8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy.
					V[X]=V[X] & V[Y];
					PC += 2;
					break;

				case 0x3: //8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy.
					V[X]=V[X] ^ V[Y];
					PC += 2;
					break;

				case 0x4: // 8xy4 - ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry.
				{
					uint16_t sum = V[X] + V[Y];
					uint8_t carry = (sum > 0xFF) ? 1 : 0;
					V[X] = sum & 0xFF;
					V[0xF] = carry;
					PC += 2;
					break;
				}

				case 0x5: // 8xy5 - SUB Vx, Vy:  Set Vx = Vx - Vy, set VF = NOT borrow.
				{
					uint8_t notborrow = (V[X] >= V[Y]) ? 1 : 0;
					uint8_t diff = V[X] - V[Y];
					V[X] = diff;
					V[0xF] = notborrow;
					PC += 2;
					break;
				}

				case 0x6: //8xy6 - SHR Vx {, Vy}: Set Vx = Vx SHR 1.
					uint8_t odd = (V[X] & 0x01 == 1) ? 1 : 0;
					uint8_t div = V[X]/2;
					V[X] = div;
					V[0xF] = odd;
					PC += 2;
					break;

				case 0x7: //8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow.
					uint8_t notborrow = (V[Y] >= V[X]) ? 1 : 0;
					uint8_t diff = V[Y] - V[X];
					V[X] = diff;
					V[0xF] = notborrow;
					PC += 2;
					break;

				case 0xE: //8xyE - SHL Vx {, Vy}: Set Vx = Vx SHL 1.
					uint8_t flag = (V[X] & 0x80 == 1) ? 1 : 0;
					uint8_t mult = V[X]*2;
					V[X] = mult;
					V[0xF] = flag;
					PC += 2;
					break;

				default:
					break;
	        }
	        break;

	    case 0x9000: //9xy0 - SNE Vx, Vy: Skip next instruction if Vx != Vy.
			if(V[X] != V[Y]){
				PC += 2;
			}
			PC += 2;
			break;

		case 0xA000: //Annn - LD I, addr: Set I = nnn.
			I = NNN;
			PC += 2;
			break;

		case 0xB000: //Bnnn - JP V0, addr: Jump to location nnn + V0.
			PC = NNN + V[0];
			PC += 2;
			break;

		case 0xC000: //Cxkk - RND Vx, byte: Set Vx = random byte AND kk.
			uint8_t random = rand() & 0xFF;
			V[X] = rand() & NN;
			PC += 2;
			break;

		case 0xD000: //Dxyn - DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
			if(I>0xFFF){
				fprintf(stderr, "Fatal Error: I>RAM = 0x%04X\n", PC);
			}
			else{
				chip -> V[0xF]=0;
				for (int i=0; i<N; i++){
					//---------------------------------
					//BURAYI DA DAHA YAPMADIM SONRA YAP
					//---------------------------------
					uint8_t sprite = ram[I+i];
					for (int j=0; j<8; j++){
						if (display[V[X]+j][V[Y]+i] == 1 && sprite>>(7-j)==1){V[0xF] = 1;}
						display[V[X]+j][V[Y]+i]=(sprite >> j)^display[V[X]+j][V[Y]+i];
					}
				}
			}
			PC += 2;
			break;

	    case 0xE000:
	        switch (NN) {
				case 0x9E: //Ex9E - SKP Vx: Skip next instruction if key with the value of Vx is pressed.
					if (key[V[X]] == 1){
						PC+=2;
					}
					PC += 2;
					break;
				case 0xA1: //ExA1 - SKNP Vx: Skip next instruction if key with the value of Vx is not pressed.
					if (key[V[X]] == 0){
						PC+=2;
					}
					PC += 2;
					break;
	            default:
					break;
	        }
	        break;

	    case 0xF000:
	        switch (NN) {
				case 0x07: //Fx07 - LD Vx, DT: Set Vx = delay timer value.
					V[X] = DT;
					PC += 2;
					break;

				case 0x0A: //Fx0A - LD Vx, K: Wait for a key press, store the value of the key in Vx.
                    while (true) {
                        for (int i = 0; i < 15; i++) {
                            if (key[i]) {
                                V[x] = i;
                                break;
                            }
                        }
                    }
                    PC += 2;
                    break;

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
					//---------------------------------
					//BURAYI DA DAHA YAPMADIM SONRA YAP
					//---------------------------------
					break;

				case 0x33: //Fx33 - LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2.

					break;
	            case 0x55: sprintf(op, "LD [I], V%X", X); break;
	            case 0x65: sprintf(op, "LD V%X, [I]", X); break;
	            default:   sprintf(op, "???"); break;
	        }
	        break;

	    default: sprintf(op, "???"); break;
	}

}
