#include <stdio.h>
#include "cpu.h"
#include "types.h"

#define INSTR_R(op, rd, rs1, rs2) \
    (((uint)(op) << 12) | ((uint)(rd) << 8) | ((uint)(rs1) << 4) | ((uint)(rs2)))

#define INSTR_I(op, rd, rs, imm) \
    (((uint)(op) << 12) | ((uint)(rd) << 8) | ((uint)(rs) << 4) | ((uint)((imm) & 0xF)))

#define INSTR_J(addr) \
    (((uint)(OP_J) << 12) | ((uint)(addr) & 0x0FFF))

int main(void) {
    CPU cpu;
    cpu_init(&cpu);

    cpu.data_mem[0] = 1;
    cpu.data_mem[1] = 4;
    cpu.data_mem[2] = 1;

    uint programa[] = {
        INSTR_I(OP_LW, 1, 0, 0),
        INSTR_I(OP_LW, 3, 0, 1),
        INSTR_I(OP_LW, 4, 0, 2),
        INSTR_R(OP_ADD, 2, 0, 0),
        INSTR_R(OP_ADD, 2, 2, 1),
        INSTR_R(OP_ADD, 1, 1, 4),
        INSTR_I(OP_BEQ, 1, 3, 1),
        INSTR_J(0x04),
        INSTR_I(OP_SW, 0, 2, 5),
        0x0000
    };

    uint cantidad = sizeof(programa) / sizeof(programa[0]);

    printf("\n=== MICROPROCESADOR EN C ===\n");
    printf("Programa: suma 1+2+3 usando bucle (add, lw, sw, beq, j)\n");
    printf("Resultado esperado en mem[5]: 6\n\n");

    cpu_load_program(&cpu, programa, cantidad);
    cpu_run(&cpu);

    printf("\nVerificacion: mem[5] = %u  (esperado: 6)\n", cpu.data_mem[5]);
    return 0;
}
