#include "chip8.h"
#include <string.h>
#include <stdio.h>

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

int init(chip8 *chip){
	//CHIP 0'a initialize ediliyor.
	memset(chip, 0, sizeof(chip8));
	//RAM ve PC kuruluyor.
	memcpy(chip->ram+0x050, font, sizeof(font));
	chip->PC=0x200;
	return 0;
}

int load_rom(chip8 *chip, char *rom_file_addr){
	FILE *fp=fopen(rom_file_addr, "rb");
	fseek(fp, 0 , SEEK_END);
	long file_size=ftell(fp);
	rewind(fp);

	if (file_size>0xFFF-0x1FF){
		return -1;
	}
	else{
		memcpy(chip->ram+0x200, fp, file_size);
		return 0;
	}
	
}

void dump(chip8 *chip){

	//Debug için kullanılmak üzere istenen çıktılar yazılıyor.
	printf("PC = 0x%04X	SP = 0x%02X	I = 0x%04X\n", chip->PC, chip->SP, chip->I);

	for(int j=0; j<2;j++){
		for (int i=0; i<8;i++){
			printf("V%01X=%02X ",i+j*8, chip->V[i+j*8]);
		}
		printf("\n");
	}

	printf("DT=%02X	ST=%02X\n",chip->DT, chip->ST);
	
	//STACK EKLENECEK!
}

int cycle(chip8 *chip){
	//opcode 2 byte RAM verisi okuyarak elde edildi (16 bit uzunluğunda)
	//FETCH
	uint16_t op = (chip->ram[c->PC] << 8) | chip->ram[c->PC + 1];
	chip->PC += 2;

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
					memset(chip->display, 0, sizeof(chip->display));
	            	break;
	            case 0xEE://RET: Return from a subroutine.
					if(chip->SP==0){
						fprintf(stderr, "Fatal Error: Stack Underflow at PC = 0x%04X\n", chip->PC);
						return -1;
					}
	            	chip->SP--;				
	            	chip->PC=chip->stack[chip->SP];
	            	break;
	            default:
	            	break;
	        }
	        break;

	    case 0x1000: //JP addr: Jump to location nnn.
			chip->PC=NNN;
	    	break;

	    case 0x2000: //CALL addr: Call subroutine at nnn.
			if(chip->SP>=15){
				fprintf(stderr, "Fatal Error: Stack Overflow at PC = 0x%04X\n", chip->PC);
				return -1;
			chip->stack[chip->SP]=chip->PC;
			chip->SP++;
			chip->PC=NNN;
	    	break;

	    case 0x3000: //SE Vx, byte: Skip next instruction if Vx = kk.
			BURADA KALDIN
	    	break;

	    case 0x4000: sprintf(op, "SNE V%X, 0x%02X", X, NN); break;
	    case 0x5000: sprintf(op, "SE V%X, V%X", X, Y); break;
	    case 0x6000: sprintf(op, "LD V%X, 0x%02X", X, NN); break;
	    case 0x7000: sprintf(op, "ADD V%X, 0x%02X", X, NN); break;

	    case 0x8000:
	        switch (N) {
	            case 0x0: sprintf(op, "LD V%X, V%X", X, Y); break;
	            case 0x1: sprintf(op, "OR V%X, V%X", X, Y); break;
	            case 0x2: sprintf(op, "AND V%X, V%X", X, Y); break;
	            case 0x3: sprintf(op, "XOR V%X, V%X", X, Y); break;
	            case 0x4: sprintf(op, "ADD V%X, V%X", X, Y); break;
	            case 0x5: sprintf(op, "SUB V%X, V%X", X, Y); break;
	            case 0x6: sprintf(op, "SHR V%X", X); break;
	            case 0x7: sprintf(op, "SUBN V%X, V%X", X, Y); break;
	            case 0xE: sprintf(op, "SHL V%X", X); break;
	            default:  sprintf(op, "???"); break;
	        }
	        break;

	    case 0x9000: sprintf(op, "SNE V%X, V%X", X, Y); break;
	    case 0xA000: sprintf(op, "LD I, 0x%03X", NNN); break;
	    case 0xB000: sprintf(op, "JP V0, 0x%03X", NNN); break;
	    case 0xC000: sprintf(op, "RND V%X, 0x%02X", X, NN); break;
	    case 0xD000: sprintf(op, "DRW V%X, V%X, %X", X, Y, N); break;

	    case 0xE000:
	        switch (NN) {
	            case 0x9E: sprintf(op, "SKP V%X", X); break;
	            case 0xA1: sprintf(op, "SKNP V%X", X); break;
	            default:   sprintf(op, "???"); break;
	        }
	        break;

	    case 0xF000:
	        switch (NN) {
	            case 0x07: sprintf(op, "LD V%X, DT", X); break;
	            case 0x0A: sprintf(op, "LD V%X, K", X); break;
	            case 0x15: sprintf(op, "LD DT, V%X", X); break;
	            case 0x18: sprintf(op, "LD ST, V%X", X); break;
	            case 0x1E: sprintf(op, "ADD I, V%X", X); break;
	            case 0x29: sprintf(op, "LD F, V%X", X); break;
	            case 0x33: sprintf(op, "LD B, V%X", X); break;
	            case 0x55: sprintf(op, "LD [I], V%X", X); break;
	            case 0x65: sprintf(op, "LD V%X, [I]", X); break;
	            default:   sprintf(op, "???"); break;
	        }
	        break;

	    default: sprintf(op, "???"); break;
	}

}
