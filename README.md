# CHIP-8 Emulator

Bu proje bilgiasayar mimarisi temellerini öğrenmek ve C dilinde pratik kazanmak amacıyla oluşturulmuştur.
## Kurulum ve Çalıştırma

```sh
make                           # Derle
./chip8 roms/IBM_Logo.ch8     # ROM'u çalıştır
```

Pencereyi kapatarak veya ESC'ye basarak çıkılır.

## Kontroller

CHIP-8 tuşları modern klavyeye eşlenmiştir:

```
CHIP-8 Keypad          Modern Keyboard
┌───┬───┬───┬───┐      ┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ C │      │ 1 │ 2 │ 3 │ 4 │
├───┼───┼───┼───┤      ├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ D │      │ Q │ W │ E │ R │
├───┼───┼───┼───┤      ├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ E │      │ A │ S │ D │ F │
├───┼───┼───┼───┤      ├───┼───┼───┼───┤
│ A │ 0 │ B │ F │      │ Z │ X │ C │ V │
└───┴───┴───┴───┘      └───┴───┴───┴───┘
```

## Dosyalar

- `chip8.c` - CPU emülatörü (tüm opcode'lar, bellek, SDL2 entegrasyon)
- `main.c` - Ana döngü (input, timing, display)
- `inspector.c` - ROM disassembler

## Gereksinimler

- GCC (C99)
- SDL2 dev libraries

ROM Disassembler (opsiyonel):
```sh
gcc src/inspector.c -Wall -Wextra -std=c99 -o inspector
./inspector roms/IBM_Logo.ch8
```

## Mimari

**Bellek Yapısı:**
```
0x000 ─ 0x1FF    Sistem alanı (font data)
0x200 ─ 0xFFF    Program kodu
```

**CPU Bileşenleri:**
```
Registerler:  V0-VF (8-bit) │ I (16-bit) │ PC (16-bit) │ SP (4-bit)
Bellek:       4096 byte RAM
Stack:        16 seviye
Timerler:     DT, ST (60Hz'de azalır)
```

**Opcode Mimarı:**
- 35 farklı talimat (0x0000-0xFFFF aralığında)
- Her opcode 16-bit (2 byte) uzunlukta
- Fetch-Decode-Execute döngüsü

**Çalışma Hızı:**
- 60Hz ekran yenileme
- Frame başına 10 CPU cycle (~600 cycle/sec)



