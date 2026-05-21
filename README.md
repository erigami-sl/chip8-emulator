# CHIP-8 Emulator

This is an educational CHIP-8 emulator project written in C. The goal of the
project is to learn how a simple virtual machine works by implementing the core
parts manually, without relying on an existing emulator framework.

> This is a temporary README generated with AI.

## Status

The project is still under development and is not finished yet. Some parts of
the emulator are implemented, while the CPU cycle and opcode execution logic are
still incomplete.

Current work includes:

- CHIP-8 memory/register structure
- Font data initialization
- ROM loading into memory
- Basic debug dump output
- A small ROM inspector/disassembler utility
- Early opcode decoding and execution work

## Project Structure

```text
.
├── include/chip8.h      # CHIP-8 structure and function declarations
├── src/chip8.c          # Core emulator logic
├── src/main.c           # Temporary test entry point
├── src/inspector.c      # ROM inspection/disassembly helper
├── roms/IBM_Logo.ch8    # Example CHIP-8 ROM
└── Makefile             # Basic build command
```

## Building

The main emulator is intended to be built with:

```sh
make
```

At the moment, the main emulator may not compile successfully because the CPU
cycle and opcode execution work is still in progress.

The ROM inspector can be built manually if needed:

```sh
gcc src/inspector.c -Wall -Wextra -std=c99 -o inspector
```

Example usage:

```sh
./inspector roms/IBM_Logo.ch8
```

## Notes

This project is being developed for learning purposes. The code is intentionally
hand-written as part of the process of understanding CHIP-8 memory, registers,
opcodes, and execution flow.
