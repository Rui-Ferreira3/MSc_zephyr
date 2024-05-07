#ifndef COMMON_H
#define COMMON_H

#define ACCELERATOR_BASE_ADDRESS            0x90000000
#define MEMORY_BASE_ADDRESS                 0x04000000
#define A1_BASE_ADDRESS                     MEMORY_BASE_ADDRESS
#define A1_SIZE                             sizeof(float)*W1_COLS
#define A2_BASE_ADDRESS                     A1_BASE_ADDRESS + A1_SIZE
#define A2_SIZE                             sizeof(float)*W2_COLS
#define YHAT_BASE_ADDRESS                   A2_BASE_ADDRESS + A2_SIZE
#define YHAT_SIZE                           sizeof(float)*W3_COLS
#define COLUMN_BASE_ADDRESS                 0x05000000

#define DIGITS 256
#define DIGIT_SIZE 784
#define W1_ROWS DIGIT_SIZE
#define W1_COLS 128
#define W2_ROWS W1_COLS
#define W2_COLS 64
#define W3_ROWS W2_COLS
#define W3_COLS 10

#define MAX_MATRIX_SIZE 1024

#define CONFIG_THREAD_MAX_NAME_LEN 100

/* size of stack area used by each thread */
#define STACKSIZE 1024

#endif //COMMON_H