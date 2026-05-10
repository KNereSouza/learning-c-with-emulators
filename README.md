# CHIP-8 Emulator

A CHIP-8 emulator written in C from scratch, as a study project for low-level programming and computer architecture.

Renders output as ASCII in the terminal. Reads keyboard input directly from the controlling terminal (raw, non-blocking).

## Status

**v0.3** — Keyboard input added (`EX9E`, `EXA1`, `FX0A`). Still missing timers and the `Fx`-family memory ops.

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
| `8XY0` | LD Vx, Vy |
| `8XY1` | OR Vx, Vy |
| `8XY2` | AND Vx, Vy |
| `8XY3` | XOR Vx, Vy |
| `8XY4` | ADD Vx, Vy — with carry flag in VF |
| `8XY5` | SUB Vx, Vy — with NOT-borrow flag in VF |
| `8XY6` | SHR Vx — VF = lost LSB |
| `8XY7` | SUBN Vx, Vy — VF = NOT-borrow flag |
| `8XYE` | SHL Vx — VF = lost MSB |
| `9XY0` | SNE Vx, Vy — skip next if Vx != Vy |
| `ANNN` | LD I, NNN — set index register |
| `BNNN` | JP V0, NNN — jump to V0 + NNN |
| `CXNN` | RND Vx, NN — Vx = random byte AND NN |
| `DXYN` | DRW Vx, Vy, N — draw sprite |
| `EX9E` | SKP Vx — skip next if key Vx is pressed |
| `EXA1` | SKNP Vx — skip next if key Vx is not pressed |
| `FX0A` | LD Vx, K — block until any key is pressed, store it in Vx |

Next up: timers (`FX07`, `FX15`, `FX18`) and the rest of the `Fx` family (`FX1E`, `FX29`, `FX33`, `FX55`, `FX65`).

## Building

Requires a C11 compiler (gcc or clang) on a POSIX system (uses `termios.h`).

```bash
gcc -Wall -Wextra -std=c11 chip8.c -o chip8
```

## Running

```bash
./chip8                        # loads roms/IBM Logo.ch8 by default
./chip8 roms/Snow.ch8          # or pass any ROM path
```

Exit with `Ctrl+C` (the SIGINT handler restores the terminal).

## Keyboard layout

The CHIP-8 has a 16-key hex keypad. It is mapped to the left side of a QWERTY keyboard:

```
 CHIP-8       Keyboard
 1 2 3 C      1 2 3 4
 4 5 6 D  →   Q W E R
 7 8 9 E      A S D F
 A 0 B F      Z X C V
```

## Layout

```
chip8.c            emulator source (single-file)
roms/              test ROMs
├── IBM Logo.ch8   classic first ROM (draws the IBM logo)
└── Snow.ch8       small demo using CXNN (random pixel field)
```
