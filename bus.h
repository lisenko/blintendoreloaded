#pragma once
#include <stdint.h>

#include "6502.h"

void memw(struct cpu *cpu, uint16_t addr, uint8_t data);
uint8_t memr(struct cpu *cpu, uint16_t addr);