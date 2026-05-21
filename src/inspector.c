#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]){

	uint8_t ram[4096]={0};

	if (argc < 2) {
	        printf("Kullanim: %s <dosya_adi>\n", argv[0]);
	        return 1;
	    }

	FILE *fp = fopen(argv[1], "rb");

	if (fp == NULL) {
	        perror("Dosya acilamadi");
	        return 1;
	    }
	
	fseek(fp, 0, SEEK_END);
	long file_size=ftell(fp);
	rewind(fp);

	if (file_size > 3584 || file_size == 0) {
	        fclose(fp);
	        return -1;
	    }
	
	size_t bytes_read = fread(&ram[0x200], 1, file_size, fp);

	fclose(fp);

	if ((long)bytes_read!=file_size){
		return -1;
	}

	//Veri RAM'e alındı
	uint16_t op_code;
	uint16_t addr;
	char op[32];
	for (int i=0x200; i < (0x200 + file_size); i+=2){
		if (i+1==file_size){
			op_code = (ram[i]<<8);
		}
		else{
			op_code = (ram[i] << 8) | ram[i+1];
		}
		addr = i;
		uint8_t  X   = (op_code & 0x0F00) >> 8; 
		uint8_t  Y   = (op_code & 0x00F0) >> 4;  
		uint8_t  N   = (op_code & 0x000F);      
		uint8_t  NN  = (op_code & 0x00FF);      
		uint16_t NNN = (op_code & 0x0FFF);       

		switch(op_code & 0xF000) {
		    case 0x0000:
		        switch (NN) {
		            case 0xE0: sprintf(op, "CLS"); break;
		            case 0xEE: sprintf(op, "RET"); break;
		            default:   sprintf(op, "SYS 0x%03X", NNN); break;
		        }
		        break;

		    case 0x1000: sprintf(op, "JP 0x%03X", NNN); break;
		    case 0x2000: sprintf(op, "CALL 0x%03X", NNN); break;
		    case 0x3000: sprintf(op, "SE V%X, 0x%02X", X, NN); break;
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

		
		printf("0x%X: 0x%04X - %s\n",addr, op_code, op);
	}

}

