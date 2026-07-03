#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

#define DATA_MEM_SIZE 1024
#define PROG_MEM_SIZE 1024
#define NUM_REGS 16

#define OP_ADD 0x0
#define OP_LW 0x1
#define OP_SW 0x3
#define OP_BEQ 0x4
#define OP_J 0xE

#define REG_ZERO 0
#define REG_RA 10
#define REG_GP 11
#define REG_SP 12
#define REG_FP 13
#define REG_A0 14
#define REG_A1 15

#endif
