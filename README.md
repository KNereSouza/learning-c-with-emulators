# CHIP-8 Emulator

A CHIP-8 emulator written in C from scratch, as a study project for low-level programming and computer architecture.

Renders output as ASCII in the terminal.

## Status

**v0.1** — Runs the *IBM Logo* ROM.

## Implemented opcodes

| Opcode | Description |
|--------|-------------|
| `00E0` | CLS — clear the display |
| `00EE` | RET — return from subroutine |
| `1NNN` | JP — unconditional jump |
| `2NNN` | CALL — call subroutine |
| `3XNN` | SE Vx, NN — skip next if Vx == NN |
| `4XNN` | SNE Vx, NN — skip next if Vx != NN |
| `5XY0` | SE Vx, Vy — skip next if Vx == Vy |
| `6XNN` | LD Vx, NN — load immediate |
| `7XNN` | ADD Vx, NN — add immediate |
| `9XY0` | SNE Vx, Vy — skip next if Vx != Vy |
| `ANNN` | LD I, NNN — set index register |
| `DXYN` | DRW Vx, Vy, N — draw sprite |

Next up: `8XYZ` ALU family, `CXNN` (random), keyboard (`EXnn`), and timers (`FXnn`).

## Building

Requires a C11 compiler (gcc or clang) on a POSIX system.

```bash
gcc -Wall -Wextra -std=c11 chip8.c -o chip8
```

## Running

```bash
./chip8
```

Loads `roms/IBM Logo.ch8` by default. Exit with `Ctrl+C`.

## Layout

```
chip8.c            emulator source (single-file)
roms/              test ROMs
└── IBM Logo.ch8
```
