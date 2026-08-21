#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define FRAME_DELAY 16

#define CYCLES_PER_FRAME 10

int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Kullanim: %s <ROM_DOSYASI.ch8>\n", argv[0]);
        return EXIT_FAILURE;
    }

    init(); // RAM'i sıfırla, fontları yükle, PC'yi 0x200'e ayarla

    if (load_rom(argv[1]) != 0) {
        fprintf(stderr, "Hata: ROM dosyasi yuklenemedi! (%s)\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (!init_sdl()) {
        fprintf(stderr, "Hata: SDL alt sistemleri baslatilamadi!\n");
        return EXIT_FAILURE;
    }

    bool quit = false;
    uint32_t last_tick = SDL_GetTicks();

    while (!quit) {
        handle_input(&quit, key);

        for (int i = 0; i < CYCLES_PER_FRAME; i++) {
            cycle();
        }

        if (DT > 0) {
            DT--; // Delay Timer sıfıra doğru sayar
        }

        if (ST > 0) {
            ST--; // Sound Timer sıfıra doğru sayar
            set_sound(true);  // ST 0'dan büyükse bip sesi çıkar
        } else {
            set_sound(false); // ST 0 ise sesi sustur
        }

        draw_display(display);

        uint32_t current_tick = SDL_GetTicks();
        uint32_t elapsed_time = current_tick - last_tick;

        if (elapsed_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - elapsed_time);
        }
        last_tick = SDL_GetTicks();
    }

    cleanup_sdl();
    return EXIT_SUCCESS;
}
