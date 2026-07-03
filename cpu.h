#ifndef CPU_H
#define CPU_H

#include "types.h"

typedef struct {
    uint pc;
    uint regs[NUM_REGS];
    uint data_mem[DATA_MEM_SIZE];
    uint prog_mem[PROG_MEM_SIZE];
    int running;
} CPU;

void cpu_init(CPU *cpu);
void cpu_load_program(CPU *cpu, uint *instrucciones, uint cantidad);
void cpu_step(CPU *cpu);
void cpu_run(CPU *cpu);
void cpu_dump(const CPU *cpu);

#endif
