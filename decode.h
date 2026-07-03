#ifndef DECODE_H
#define DECODE_H

#include "types.h"

typedef struct {
    uint opcode;
    uint rd;
    uint rs1;
    uint rs2;
    uint imm;
    uint addr;
    int imm_s;
} Instruccion;

Instruccion decode(uint raw);
void decode_print(const Instruccion *instr);

#endif
