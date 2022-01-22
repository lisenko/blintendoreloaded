#include "bus.h"
#include "6502.h"

#include <stdint.h>

uint8_t tempmem[65536];

void memw(struct cpu *cpu, uint16_t addr, uint8_t data) {
    switch(addr) {
        case 0x0 ... 0x7FF:
            cpu->wram[addr] = data;
            //return;
        default:
            tempmem[addr] = data;
            return;
    }
}

uint8_t memr(struct cpu *cpu, uint16_t addr) {
    switch(addr) {
        case 0x0 ... 0x7FF:
            return cpu->wram[addr];
        default:
            return 0xFF;
    }
}