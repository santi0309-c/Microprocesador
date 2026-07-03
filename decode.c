#include "decode.h"
#include <stdio.h>

Instruccion decode(uint raw) {
    Instruccion instr;

    instr.opcode = (raw >> 12) & 0xF;
    instr.rd = (raw >> 8) & 0xF;
    instr.rs1 = (raw >> 4) & 0xF;
    instr.rs2 = raw & 0xF;
    instr.imm = raw & 0xF;

    if (instr.imm & 0x8) {
        instr.imm_s = (int)(instr.imm | 0xFFFFFFF0);
    } else {
        instr.imm_s = (int)instr.imm;
    }

    instr.addr = raw & 0x0FFF;
    return instr;
}

void decode_print(const Instruccion *instr) {
    printf("  [DECODE] opcode=0x%X  rd=r%u  rs1=r%u  rs2=r%u  imm=%u (signed=%d)  addr=0x%03X\n",
           instr->opcode,
           instr->rd,
           instr->rs1,
           instr->rs2,
           instr->imm,
           instr->imm_s,
           instr->addr);
}
