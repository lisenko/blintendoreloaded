#include "6502.h"
#include "bus.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* SR Flags (bit 7 to bit 0)
N	Negative
V	Overflow
-	ignored
B	Break
D	Decimal (use BCD for arithmetics)
I	Interrupt (IRQ disable)
Z	Zero
C	Carry */

uint8_t *_rom;
struct cpu *_cpu;

uint8_t op1() {
    return _rom[_cpu->pc + 1];
}

uint8_t op2() {
    return _rom[_cpu->pc + 1];
}

void flags_zn(struct cpu *cpu, uint8_t val) {
    if (val == 0) {
        cpu->sr &= ~0b10;
    }
    if (val & 0b10000000) {
        cpu->sr |= 0b10000000;
    }
}

void clc(struct cpu *cpu) {
    cpu->sr &= ~0b1;
    cpu->pc++;
    cpu->cycles += 2;
}

void sec(struct cpu *cpu) {
    cpu->sr |= 0b1;
    cpu->pc++;
    cpu->cycles += 2;
}

void cli(struct cpu *cpu) {
    cpu->sr &= ~0b100;
    cpu->pc++;
    cpu->cycles += 2;
}

void sei(struct cpu *cpu) {
    cpu->sr |= 0b100;
    cpu->pc++;
    cpu->cycles += 2;
}

void clv(struct cpu *cpu) {
    cpu->sr &= ~0b1000000;
    cpu->pc++;
    cpu->cycles += 2;
}

void cld(struct cpu *cpu) {
    cpu->sr &= ~0b1000;
    cpu->pc++;
    cpu->cycles += 2;
}

void sed(struct cpu *cpu) {
    cpu->sr |= 0b1000;
    cpu->pc++;
    cpu->cycles += 2;
}

void lda(struct cpu *cpu, uint8_t byte) {
    cpu->a = byte;
    //update pc in opcode
    //update cycles in opcode
    flags_zn(cpu, cpu->a);
};

void adc(struct cpu *cpu, uint8_t byte) {
    uint16_t result = cpu->a + byte + (cpu->sr & 1 ? 1 : 0);
    // TODO V
    if (result > 256) {
        cpu->sr |= 0b1;
        result -= 256;
    }
    cpu->a = (uint8_t)result;
    //update pc in opcode
    //update cycles in opcode
    flags_zn(cpu, cpu->a);
};

void sbc(struct cpu *cpu, uint8_t byte) {
    uint16_t result = 0xFF + cpu->a - byte + (cpu->sr & 1 ? 0 : 1);
    // TODO V
    if (result > 256) {
        cpu->sr |= 0b1;
        result -= 256;
    }
    cpu->a = (uint8_t)result;
    //update pc in opcode
    //update cycles in opcode
    flags_zn(cpu, cpu->a);
};

//TAX, TAY, TSX, TXA, TYA
void trans(struct cpu *cpu, uint8_t *src, uint8_t *dest) { //rights
    *dest = *src;
    cpu->pc++;
    cpu->cycles += 2;
    flags_zn(cpu, *dest);
}

void txs(struct cpu *cpu) {
    cpu->sp = cpu->x;
    cpu->pc++;
    cpu->cycles += 2;
}

void inx(struct cpu *cpu) {
    cpu->x++;
    cpu->pc++;
    cpu->cycles += 2;
    flags_zn(cpu, cpu->x);
}

void iny(struct cpu *cpu) {
    cpu->y++;
    cpu->pc++;
    cpu->cycles += 2;
    flags_zn(cpu, cpu->y);
}

char * step(struct cpu *cpu, uint8_t opcode, uint8_t rom[]) {
    char *msg = malloc(100);
    _cpu = cpu;
    _rom = rom;
    switch(opcode) {
        case 0x00: // BRK
            //TODO CYKA
            return "BRK";
        case 0x18: // CLC
            clc(cpu);
            return "CLC";
        case 0x38: // SEC
            sec(cpu);
            return "SEC";
        case 0x58: // CLI
            cli(cpu);
            return "CLI";
        case 0x65: // ADC zp
            adc(cpu, memr(cpu, op1()));
            snprintf(msg, 100, "ADC $%X", op1());
            cpu->pc += 2;
            cpu->cycles += 2;
            return msg;
        case 0x69: // ADC imm
            adc(cpu, op1());
            snprintf(msg, 100, "ADC $#%X", op1());
            cpu->pc += 2;
            cpu->cycles += 2;
            return msg;
        case 0x78: // SEI
            sei(cpu);
            return "SEI";
        case 0x85: // STA zp
            memw(cpu, op1(), cpu->a);
            snprintf(msg, 100, "STA $%X", op1());
            cpu->pc += 2;
            cpu->cycles += 3;
            return msg;
        case 0x8A: // TXA
            trans(cpu, &cpu->x, &cpu->a);
            return "TXA";
        case 0x98: // TYA
            trans(cpu, &cpu->y, &cpu->a);
            return "TYA";
        case 0x9A: // TXS
            txs(cpu);
            return "TXS";
        case 0xA8: // TAY
            trans(cpu, &cpu->a, &cpu->y);
            return "TAY";
        case 0xA5: // LDA zp
            lda(cpu, memr(cpu, op1()));
            snprintf(msg, 100, "LDA $%X", op1());
            cpu->pc += 2;
            cpu->cycles += 3;
            return msg;
        case 0xA9: // LDA imm
            lda(cpu, op1());
            snprintf(msg, 100, "LDA $#%X", op1());
            cpu->pc += 2;
            cpu->cycles += 2;
            return msg;
        case 0xAA: // TAX
            trans(cpu, &cpu->a, &cpu->x);
            return "TAX";
        case 0xAD: // LDA abs
            uint16_t addr = (op2() << 8) + op1();
            lda(cpu, memr(cpu, addr));
            snprintf(msg, 100, "LDA $%X%X", op2(), op1());
            cpu->pc += 3;
            cpu->cycles += 4;
            return msg;
        case 0xB5: // LDA zp+x
            lda(cpu, memr(cpu, rom[(op1() + cpu->x) % 256]));
            snprintf(msg, 100, "LDA $%X", op1());
            cpu->pc += 2;
            cpu->cycles += 4;
            return msg;
        case 0xB8: // CLV
            clv(cpu);
            return "CLV";
        case 0xBA: // TSX
            trans(cpu, &cpu->sp, &cpu->x);
            return "TSX";
        case 0xC8: // INY
            iny(cpu);
            return "INY";
        case 0xD8: // CLD
            cld(cpu);
            return "CLD";
        case 0xE8: // INX
            inx(cpu);
            return "INX";
        case 0xE9: // SBC imm
        
        case 0xEA: // NOP
            cpu->pc++;
            cpu->cycles += 2;
            return "NOP";
        case 0xF8: // SED
            sed(cpu);
            return "SED";
        default:
            snprintf(msg, 100, "Unknown opcode %X", opcode);
            return msg;
    }
}