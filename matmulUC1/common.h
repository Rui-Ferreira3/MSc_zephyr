#ifndef COMMON_H
#define COMMON_H

#define ACCELERATOR_BASE_ADDRESS            0x90000000

#define NUM_MATMULS 150
#define MAT1ROWS 32
#define MAT1COLS MAT1ROWS
#define MAT2COLS MAT1COLS

/* size of stack area used by each thread */
#define STACKSIZE 1024

#endif //COMMON_H