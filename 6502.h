#pragma once
#include <stdint.h>

struct cpu {
    uint64_t cycles;
    uint16_t pc;
    uint8_t sp;
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sr;
    uint8_t wram[0x800];
};

char * step(struct cpu *cpu, uint8_t opcode, uint8_t *rom);