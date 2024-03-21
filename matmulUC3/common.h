#ifndef COMMON_H
#define COMMON_H

#define ACCELERATOR_BASE_ADDRESS            0x90000000

#define NUM_MATMULS 10
#define MAT1ROWS 44
#define MAT1COLS MAT1ROWS
#define MAT2COLS MAT1COLS

#define NUM_THREADS 5

#define CONFIG_THREAD_MAX_NAME_LEN 100

/* size of stack area used by each thread */
#define STACKSIZE 1024

#endif //COMMON_H