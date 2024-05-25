#ifndef DEFINES_H
#define DEFINES_H

#define ACCELERATOR_BASE_ADDRESS            0x90000000
#define MEMORY_BASE_ADDRESS                 0x04000000

#define NUM_MULTIPLICATIONS 512
#define MAT1ROWS 32
#define MAT1COLS MAT1ROWS
#define MAT2COLS MAT1COLS

#define MATRIX1_SIZE                         MAT1ROWS*MAT1COLS*sizeof(float)
#define MATRIX2_SIZE                         MAT1COLS*MAT2COLS*sizeof(float)
#define RESULT_SW_SIZE                       MAT1ROWS*MAT2COLS*sizeof(float)
#define RESULT_POOL_SIZE                     MAT1ROWS*MAT2COLS*sizeof(float)

/* size of stack area used by each thread */
#define STACKSIZE 1024

#endif //DEFINES_H