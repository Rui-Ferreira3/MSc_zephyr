#ifndef COMMON_H
#define COMMON_H

#define ACCELERATOR_BASE_ADDRESS            0x90000000
#define MEM_BASE_ADDRESS                    0x02000000

#define ACCELERATOR_BUSY_FLAG_ADDR          0x02000600

#define NUM_MATMULS 100

/* size of stack area used by each thread */
#define STACKSIZE 1024

#define NUM_MATS 3

#define MATI_BASE_ADDRESS 0x02000000
#define MATI_ROWS 8
#define MATI_COLS 8

#define MAT1_BASE_ADDRESS 0x02000100
#define MAT1_ROWS 8
#define MAT1_COLS 8

#define MAT2_BASE_ADDRESS 0x02000200
#define MAT2_ROWS 8
#define MAT2_COLS 8

#define MAT3_BASE_ADDRESS 0x02000300
#define MAT3_ROWS 8
#define MAT3_COLS 8

#define RESULT_SW_BASE_ADDRESS 0x02000400
#define RESULT_HW_BASE_ADDRESS 0x02000500

#endif //COMMON_H