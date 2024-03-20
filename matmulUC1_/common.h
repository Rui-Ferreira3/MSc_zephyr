#ifndef COMMON_H
#define COMMON_H

#define ACCELERATOR_BASE_ADDRESS            0x90000000
#define ACCELERATOR_BUSY_FLAG_ADDR          0x02000600

#define NUM_MATMULS 10
#define MAT1ROWS 64
#define MAT1COLS MAT1ROWS
#define MAT2COLS MAT1COLS

/* size of stack area used by each thread */
#define STACKSIZE 1024

#endif //COMMON_H