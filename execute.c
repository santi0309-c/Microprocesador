#include "execute.h"
#include <stdio.h>

static void enforce_zero_reg(CPU *cpu) {
    cpu->regs[REG_ZERO] = 0;
}

void exec_add(CPU *cpu, const Instruccion *instr) {
    uint resultado = cpu->regs[instr->rs1] + cpu->regs[instr->rs2];

    cpu->regs[instr->rd] = resultado;
    enforce_zero_reg(cpu);

    printf("  [ADD] r%u = r%u(%u) + r%u(%u) = %u\n",
           instr->rd,
           instr->rs1, cpu->regs[instr->rs1],
           instr->rs2, cpu->regs[instr->rs2],
           resultado);

    cpu->pc++;
}

void exec_lw(CPU *cpu, const Instruccion *instr) {
    uint direccion = cpu->regs[instr->rs1] + instr->imm;

    if (direccion >= DATA_MEM_SIZE) {
        fprintf(stderr, "  [LW] ERROR: dirección %u fuera de rango (max %d)\n",
                direccion, DATA_MEM_SIZE - 1);
        cpu->running = 0;
        return;
    }

    cpu->regs[instr->rd] = cpu->data_mem[direccion];
    enforce_zero_reg(cpu);

    printf("  [LW] r%u = mem[r%u(%u) + %u] = mem[%u] = %u\n",
           instr->rd,
           instr->rs1, cpu->regs[instr->rs1],
           instr->imm,
           direccion,
           cpu->regs[instr->rd]);

    cpu->pc++;
}

void exec_sw(CPU *cpu, const Instruccion *instr) {
    uint direccion = cpu->regs[instr->rd] + instr->imm;

    if (direccion >= DATA_MEM_SIZE) {
        fprintf(stderr, "  [SW] ERROR: dirección %u fuera de rango (max %d)\n",
                direccion, DATA_MEM_SIZE - 1);
        cpu->running = 0;
        return;
    }

    cpu->data_mem[direccion] = cpu->regs[instr->rs1];

    printf("  [SW] mem[r%u(%u) + %u] = mem[%u] = r%u(%u)\n",
           instr->rd, cpu->regs[instr->rd],
           instr->imm,
           direccion,
           instr->rs1, cpu->regs[instr->rs1]);

    cpu->pc++;
}

void exec_beq(CPU *cpu, const Instruccion *instr) {
    if (cpu->regs[instr->rd] == cpu->regs[instr->rs1]) {
        int nuevo_pc = (int)(cpu->pc + 1) + instr->imm_s;

        printf("  [BEQ] r%u(%u) == r%u(%u): SALTO  PC %u -> %d (offset=%d)\n",
               instr->rd, cpu->regs[instr->rd],
               instr->rs1, cpu->regs[instr->rs1],
               cpu->pc + 1, nuevo_pc,
               instr->imm_s);

        if (nuevo_pc < 0 || nuevo_pc >= (int)PROG_MEM_SIZE) {
            fprintf(stderr, "  [BEQ] ERROR: PC resultante %d fuera de rango\n", nuevo_pc);
            cpu->running = 0;
            return;
        }

        cpu->pc = (uint)nuevo_pc;
    } else {
        printf("  [BEQ] r%u(%u) != r%u(%u): no salta\n",
               instr->rd, cpu->regs[instr->rd],
               instr->rs1, cpu->regs[instr->rs1]);
        cpu->pc++;
    }
}

void exec_j(CPU *cpu, const Instruccion *instr) {
    printf("  [J] Salto absoluto: PC %u -> %u\n", cpu->pc, instr->addr);

    if (instr->addr >= PROG_MEM_SIZE) {
        fprintf(stderr, "  [J] ERROR: dirección %u fuera de rango\n", instr->addr);
        cpu->running = 0;
        return;
    }

    cpu->pc = instr->addr;
}
