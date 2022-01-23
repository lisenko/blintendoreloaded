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

uint8_t *rom;
struct cpu *cpu;

uint8_t op1() {
    return rom[cpu->pc + 1];
}

uint8_t op2() {
    return rom[cpu->pc + 1];
}

void iimpl() {
    cpu->pc++;
    cpu->cycles += 2;
}

void iimm() {
    cpu->pc += 2;
    cpu->cycles += 2;
}

void iabsx(uint8_t byte) {
    cpu->pc += 3;
    cpu->cycles += 4;
    if (byte < cpu->x) cpu->cycles++;
}

void iabsy(uint8_t byte) {
    cpu->pc += 3;
    cpu->cycles += 4;
    if (byte < cpu->y) cpu->cycles++;
}

void iindx() {
    cpu->pc += 2;
    cpu->cycles += 6;
}

void iindy(uint8_t byte) {
    cpu->pc += 2;
    cpu->cycles += 5;
    if (byte < cpu->y) cpu->cycles++;
}

void flags_zn(uint8_t val) {
    if (val == 0) {
        cpu->sr |= 0b10;
    } else {
        cpu->sr &= ~0b10;
    }
    if (val & 0b10000000) {
        cpu->sr |= 0b10000000;
    } else {
        cpu->sr &= ~0b10000000;
    }
}

void clc() {
    cpu->sr &= ~0b1;
    iimpl();
}

void sec() {
    cpu->sr |= 0b1;
    iimpl();
}

void cli() {
    cpu->sr &= ~0b100;
    iimpl();
}

void sei() {
    cpu->sr |= 0b100;
    iimpl();
}

void clv() {
    cpu->sr &= ~0b1000000;
    iimpl();
}

void cld() {
    cpu->sr &= ~0b1000;
    iimpl();
}

void sed() {
    cpu->sr |= 0b1000;
    iimpl();
}

void lda(uint8_t byte) {
    cpu->a = byte;
    //update pc in opcode
    //update cycles in opcode
    flags_zn(cpu->a);
};

void adc(uint8_t byte) {
    uint16_t result = cpu->a + byte + (cpu->sr & 1 ? 1 : 0);
    cpu->sr &= ~0b1000001;
    // V = 0 when U1 + U2 >= 128 and U1 + U2 <= 383 ($17F)
    // V = 1 when U1 + U2 <  128 or  U1 + U2 >  383 ($17F)
    if ((result < 0x7f) | (result > 0x17f)) cpu->sr |= 0b1000000;
    if (result >= 0x100) cpu->sr |= 0b1;
    cpu->a = result & 0xFF;
    //update pc in opcode
    //update cycles in opcode
    flags_zn(cpu->a);
};

void sbc(uint8_t byte) {
    uint16_t result = 0xFEFF + cpu->a - byte + (cpu->sr & 1 ? 1 : 0);
    cpu->sr |= 0b1000001;
    // V = 0 when (65280 + U1) - U2 >= 65152 and (65280 + U1) - U2 <= 65407
    // V = 1 when (65280 + U1) - U2 <  65152 or  (65280 + U1) - U2 >  65407
    if (((0xFF00 + cpu->a) - byte >= 0xFE80) & ((0xFF00 + cpu->a) - byte <= 0xFF7F)) cpu->sr &= ~0b1000000;
    if (result < 0xFF00) cpu->sr &= ~0b1; 
    cpu->a = result & 0xFF;
    //update pc in opcode
    //update cycles in opcode
    flags_zn(cpu->a);
};

//TAX, TAY, TSX, TXA, TYA
void trans(uint8_t *src, uint8_t *dest) { //rights
    *dest = *src;
    iimpl();
    flags_zn(*dest);
}

void txs() {
    cpu->sp = cpu->x;
    iimpl();
}

void inx() {
    cpu->x++;
    iimpl();
    flags_zn(cpu->x);
}

void iny() {
    cpu->y++;
    iimpl();
    flags_zn(cpu->y);
}

char * step(struct cpu *_cpu, uint8_t opcode, uint8_t _rom[]) {
    char *msg = malloc(100);
    cpu = _cpu;
    rom = _rom;
    switch(opcode) {
        case 0x00: // BRK
            //TODO CYKA
            return "BRK";
        case 0x18: // CLC
            clc();
            return "CLC";
        case 0x38: // SEC
            sec();
            return "SEC";
        case 0x58: // CLI
            cli();
            return "CLI";
        case 0x65: // ADC zp
            adc(memr(cpu, op1()));
            snprintf(msg, 100, "ADC $%X", op1());
            cpu->pc += 2;
            cpu->cycles += 3;
            return msg;
        case 0x69: // ADC imm
            adc(op1());
            snprintf(msg, 100, "ADC $#%X", op1());
            iimm();
            return msg;
        case 0x78: // SEI
            sei();
            return "SEI";
        case 0x85: // STA zp
            memw(cpu, op1(), cpu->a);
            snprintf(msg, 100, "STA $%X", op1());
            cpu->pc += 2;
            cpu->cycles += 3;
            return msg;
        case 0x8A: // TXA
            trans(&cpu->x, &cpu->a);
            return "TXA";
        case 0x98: // TYA
            trans(&cpu->y, &cpu->a);
            return "TYA";
        case 0x9A: // TXS
            txs();
            return "TXS";
        case 0xA1: // LDA ind+x
            uint16_t addr = (memr(cpu, (op1() + cpu->x + 1) % 256) << 8) + memr(cpu, (op1() + cpu->x) % 256);
            lda(memr(cpu, addr));
            snprintf(msg, 100, "LDA ($%X,X)", op1());
            iindx();
            return msg;
        case 0xA5: // LDA zp
            lda(memr(cpu, op1()));
            snprintf(msg, 100, "LDA $%X", op1());
            cpu->pc += 2;
            cpu->cycles += 3;
            return msg;
        case 0xA8: // TAY
            trans(&cpu->a, &cpu->y);
            return "TAY";
        case 0xA9: // LDA imm
            lda(op1());
            snprintf(msg, 100, "LDA $#%X", op1());
            iimm();
            return msg;
        case 0xAA: // TAX
            trans(&cpu->a, &cpu->x);
            return "TAX";
        case 0xAD: // LDA abs
            uint16_t addr = (op2() << 8) + op1();
            lda(memr(cpu, addr));
            snprintf(msg, 100, "LDA $%X%X", op2(), op1());
            cpu->pc += 3;
            cpu->cycles += 4;
            return msg;
        case 0xB1: // LDA ind+y
            uint16_t addr = (memr(cpu, ((op1() + 1) % 256) << 8) + cpu->y) + memr(cpu, op1());
            lda(memr(cpu, addr));
            snprintf(msg, 100, "LDA ($%X),Y", op1());
            iindy(op1());
            return msg;
        case 0xB5: // LDA zp+x
            lda(memr(cpu, (op1() + cpu->x) % 256));
            snprintf(msg, 100, "LDA $%X,X", op1());
            cpu->pc += 2;
            cpu->cycles += 4;
            return msg;
        case 0xB8: // CLV
            clv();
            return "CLV";
        case 0xB9: // LDA abs+y
            uint16_t addr = (op2() << 8) + op1() + cpu->y;
            lda(memr(cpu, addr));
            snprintf(msg, 100, "LDA $%X%X,Y", op2(), op1());
            iabsy(op1());
            return msg;
        case 0xBA: // TSX
            trans(&cpu->sp, &cpu->x);
            return "TSX";
        case 0xBD: // LDA abs+x
            uint16_t addr = (op2() << 8) + op1() + cpu->x;
            lda(memr(cpu, addr));
            snprintf(msg, 100, "LDA $%X%X,X", op2(), op1());
            iabsx(op1());
            return msg;
        case 0xC8: // INY
            iny();
            return "INY";
        case 0xD8: // CLD
            cld();
            return "CLD";
        case 0xE8: // INX
            inx();
            return "INX";
        case 0xE9: // SBC imm
            sbc(op1());
            snprintf(msg, 100, "SBC $#%X", op1());
            iimm();
            return msg;
        case 0xEA: // NOP
            iimpl();
            return "NOP";
        case 0xF8: // SED
            sed();
            return "SED";
        default:
            snprintf(msg, 100, "Unknown opcode %X", opcode);
            return msg;
    }
}