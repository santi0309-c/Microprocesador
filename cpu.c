#include "cpu.h"
#include "decode.h"
#include "execute.h"
#include <stdio.h>
#include <string.h>

void cpu_init(CPU *cpu) {
    cpu->pc = 0;
    cpu->running = 1;

    memset(cpu->regs, 0, sizeof(cpu->regs));
    memset(cpu->data_mem, 0, sizeof(cpu->data_mem));
    memset(cpu->prog_mem, 0, sizeof(cpu->prog_mem));

    printf("[CPU] Inicializado. PC=0, %d registros, data_mem=%d words, prog_mem=%d words\n",
           NUM_REGS, DATA_MEM_SIZE, PROG_MEM_SIZE);
}

void cpu_load_program(CPU *cpu, uint *instrucciones, uint cantidad) {
    uint i;

    if (cantidad > PROG_MEM_SIZE) {
        fprintf(stderr, "[CPU] Advertencia: programa (%u instr) excede prog_mem (%d). Se trunca.\n",
                cantidad, PROG_MEM_SIZE);
        cantidad = PROG_MEM_SIZE;
    }

    for (i = 0; i < cantidad; i++) {
        cpu->prog_mem[i] = instrucciones[i];
    }

    printf("[CPU] Programa cargado: %u instrucciones\n", cantidad);
}

void cpu_step(CPU *cpu) {
    uint raw;
    Instruccion instr;

    if (cpu->pc >= PROG_MEM_SIZE) {
        printf("[CPU] PC=%u fuera de rango. Halt.\n", cpu->pc);
        cpu->running = 0;
        return;
    }

    raw = cpu->prog_mem[cpu->pc];
    printf("\n[FETCH] PC=%u  raw=0x%04X\n", cpu->pc, raw);

    if (raw == 0x0000 && cpu->pc > 0) {
        printf("[CPU] Instrucción 0x0000: HALT\n");
        cpu->running = 0;
        return;
    }

    instr = decode(raw);
    decode_print(&instr);

    switch (instr.opcode) {
        case OP_ADD:
            exec_add(cpu, &instr);
            break;
        case OP_LW:
            exec_lw(cpu, &instr);
            break;
        case OP_SW:
            exec_sw(cpu, &instr);
            break;
        case OP_BEQ:
            exec_beq(cpu, &instr);
            break;
        case OP_J:
            exec_j(cpu, &instr);
            break;
        default:
            printf("[CPU] Opcode desconocido: 0x%X en PC=%u. Halt.\n",
                   instr.opcode, cpu->pc);
            cpu->running = 0;
            break;
    }

    cpu->regs[REG_ZERO] = 0;
}

void cpu_run(CPU *cpu) {
    printf("\n========== INICIO DE EJECUCION ==========\n");

    while (cpu->running) {
        cpu_step(cpu);
    }

    printf("\n========== FIN DE EJECUCION ==========\n");
    cpu_dump(cpu);
}

void cpu_dump(const CPU *cpu) {
    uint i;
    const char *nombres[NUM_REGS] = {
        "zero", "r1",  "hexa", "r3",
        "r4",   "r5",  "r6",   "r7",
        "r8",   "r9",  "ra",   "gp",
        "sp",   "fp",  "a0",   "a1"
    };

    printf("\n--- Estado del CPU ---\n");
    printf("PC = %u\n", cpu->pc);
    printf("Registros:\n");

    for (i = 0; i < NUM_REGS; i++) {
        printf("  r%-2u (%s) = %u\n", i, nombres[i], cpu->regs[i]);
    }
}
