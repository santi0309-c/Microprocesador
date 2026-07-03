#ifndef EXECUTE_H
#define EXECUTE_H

#include "cpu.h"
#include "decode.h"

void exec_add(CPU *cpu, const Instruccion *instr);
void exec_lw(CPU *cpu, const Instruccion *instr);
void exec_sw(CPU *cpu, const Instruccion *instr);
void exec_beq(CPU *cpu, const Instruccion *instr);
void exec_j(CPU *cpu, const Instruccion *instr);

#endif
