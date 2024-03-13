#ifndef COMMON_H
#define COMMON_H

#define ACCELERATOR_BASE_ADDRESS            0x90000000
#define ACCELERATOR_BUSY_FLAG_ADDR          0x02000600

#define NUM_MATMULS 100

/* size of stack area used by each thread */
#define STACKSIZE 1024

#endif //COMMON_H