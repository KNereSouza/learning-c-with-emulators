#define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

static const uint8_t chip8_fontset[80] = {
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
    0xF0, 0x80, 0x80, 0x80, 0xF0, // B
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0x80, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

typedef struct {
    uint8_t  memory[4096];      // 4KB of RAM
    uint8_t  display[32][64];   // monochrome screen, [row][col]

    uint8_t  V[16];             // general-purpose registers V0..VF (VF doubles as flag)
    uint16_t I;                 // index register (holds memory addresses, 12 bits used)
    uint16_t PC;                // program counter (address of next instruction)

    uint16_t stack[16];         // call stack (stores return addresses)
    uint8_t  SP;                // stack pointer (index into stack)
} Chip8;

void chip8_init(Chip8 *cpu) {
    for (size_t i = 0; i < 32; i++) {
        for (size_t j = 0; j < 64; j++) {
            cpu->display[i][j] = 0;
        }
    }

    for (size_t i = 0; i < 4096; i++) {
        cpu->memory[i] = 0;
    }

    for (size_t i = 0; i < 80; i++) {
        cpu->memory[0x050 + i] = chip8_fontset[i];
    }

    for (size_t i = 0; i < 16; i++) {
        cpu->V[i] = 0;
    }

    for (size_t i = 0; i < 16; i++) {
        cpu->stack[i] = 0;
    }

    cpu->I  = 0;
    cpu->PC = 0x200;
    cpu->SP = 0;
}

void chip8_cycle(Chip8 *cpu) {
    uint8_t  msb    = cpu->memory[cpu->PC];
    uint8_t  lsb    = cpu->memory[cpu->PC + 1];
    uint16_t opcode = (msb << 8) | lsb;

    uint16_t nnn = opcode & 0x0FFF;        // 12 lower bits (address)
    uint8_t  nn  = opcode & 0x00FF;        // low byte
    uint8_t  n   = opcode & 0x000F;        // low nibble (sprite height in DXYN)
    uint8_t  x   = (opcode & 0x0F00) >> 8; // second nibble (register)
    uint8_t  y   = (opcode & 0x00F0) >> 4; // third  nibble (register)

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0:
                    for (size_t i = 0; i < 32; i++) {
                        for (size_t j = 0; j < 64; j++) {
                            cpu->display[i][j] = 0;
                        }
                    }
                    cpu->PC += 2;
                    break;
                case 0x00EE:
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                    break;
                default:
                    fprintf(stderr, "Unknown opcode: 0x%04X at PC=0x%03X\n", opcode, cpu->PC);
                    cpu->PC += 2;
                    break;
            }
            break;
        case 0x1000:
            cpu->PC = nnn;
            break;
        case 0x2000:
            cpu->stack[cpu->SP] = cpu->PC + 2;
            cpu->SP++;
            cpu->PC = nnn;
            break;
        case 0x3000:
            if (cpu->V[x] == nn) {
                cpu->PC += 4;
            } else {
                cpu->PC += 2;
            }
            break;
        case 0x4000:
            if (cpu->V[x] != nn) {
                cpu->PC += 4;
            } else {
                cpu->PC += 2;
            }
            break;
        case 0x5000:
            if (cpu->V[x] == cpu->V[y]) {
                cpu->PC += 4;
            } else {
                cpu->PC += 2;
            }
            break;
        case 0x6000:
            cpu->V[x] = nn;
            cpu->PC += 2;
            break;
        case 0x7000:
            cpu->V[x] += nn;
            cpu->PC += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0: {
                    cpu->V[x] = cpu->V[y];
                    break;
                }
                case 0x1: {
                    cpu->V[x] |= cpu->V[y];
                    break;
                }
                case 0x2: {
                    cpu->V[x] &= cpu->V[y];
                    break;
                }
                case 0x3: {
                    cpu->V[x] ^= cpu->V[y];
                    break;
                }
                case 0x4: {
                    uint16_t sum = cpu->V[x] + cpu->V[y];
                    cpu->V[x] = sum & 0xFF;
                    cpu->V[0xF] = (sum > 0xFF) ? 1 : 0;
                    break;
                }
                case 0x5: {
                    uint8_t flag = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
                    cpu->V[x] = cpu->V[x] - cpu->V[y];
                    cpu->V[0xF] = flag;
                    break;
                }
                case 0x6: {
                    uint8_t lsb = cpu->V[x] & 0x1;
                    cpu->V[x] >>= 1;
                    cpu->V[0xF] = lsb;
                    break;
                }
                case 0x7: {
                    uint8_t flag = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
                    cpu->V[x] = cpu->V[y] - cpu->V[x];
                    cpu->V[0xF] = flag;
                    break;
                }
                case 0xE: {
                    uint8_t msb = (cpu->V[x] >> 7) & 0x1;
                    cpu->V[x] <<= 1;
                    cpu->V[0xF] = msb;
                    break;
                }
                default: /* unknown 8-family */ break;
            }
            cpu->PC += 2;
            break;
        case 0x9000:
            if (cpu->V[x] != cpu->V[y]) {
                cpu->PC += 4;
            } else {
                cpu->PC += 2;
            }
            break;
        case 0xA000:
            cpu->I = nnn;
            cpu->PC += 2;
            break;
        case 0xB000:
            cpu->PC = cpu->V[0] + nnn;
            break;
        case 0xC000:
            cpu->V[x] = rand() & nn;
            cpu->PC += 2;
            break;
        case 0xD000:
            cpu->V[0xF] = 0;
            for (size_t i = 0; i < n; i++) {
                uint8_t byte = cpu->memory[cpu->I + i];
                for (size_t j = 0; j <= 7; j++) {
                    uint8_t new_pixel = (byte >> (7 - j)) & 1;
                    uint8_t py = (cpu->V[y] + i) % 32;
                    uint8_t px = (cpu->V[x] + j) % 64;
                    if (cpu->display[py][px] == 1 && new_pixel == 1) {
                        cpu->V[0xF] = 1;
                    }
                    cpu->display[py][px] ^= new_pixel;
                }
            }
            cpu->PC += 2;
            break;
        default:
            fprintf(stderr, "Unknown opcode: 0x%04X at PC=0x%03X\n", opcode, cpu->PC);
            cpu->PC += 2;
            break;
    }
}

void chip8_draw(Chip8 *cpu) {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            putchar(cpu->display[y][x] ? '#' : ' ');
        }
        putchar('\n');
    }
}

int chip8_load_rom(Chip8 *cpu, const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: could not open '%s'\n", path);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size > 4096 - 0x200) {
        fprintf(stderr, "Error: ROM too large (%ld bytes)\n", size);
        fclose(file);
        return 1;
    }

    size_t bytes_read = fread(&cpu->memory[0x200], 1, size, file);
    if ((long)bytes_read != size) {
        fprintf(stderr, "Error: expected %ld bytes, read %zu\n", size, bytes_read);
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

#define CYCLES_PER_FRAME 8
#define FRAME_DELAY_US   16666

int main(int argc, char **argv) {
    srand(time(NULL));

    const char *rom_path = (argc > 1) ? argv[1] : "roms/IBM Logo.ch8";

    Chip8 cpu;
    chip8_init(&cpu);

    if (chip8_load_rom(&cpu, rom_path) != 0) {
        return 1;
    }

    printf("\033[H\033[2J");

    for (;;) {
        for (int i = 0; i < CYCLES_PER_FRAME; i++) {
            chip8_cycle(&cpu);
        }
        printf("\033[H");
        chip8_draw(&cpu);
        usleep(FRAME_DELAY_US);
    }

    return 0;
}
