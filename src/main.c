#include "chip8.h"
#include <stdlib.h>


int main(){

	//DENEME AMAÇLII YAZILMIŞ MAİN
	chip8 *chip = (chip8*)malloc(sizeof(chip8));

	init(chip);
	load_rom(chip, "/home/cake/Documents/Projects/chip8-emulator/roms/IBM_Logo.ch8");
	dump(chip);
	free(chip);
	return 0;
}
