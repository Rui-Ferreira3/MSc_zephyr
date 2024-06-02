#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <zephyr.h>
#include <kernel.h>
#include <random/rand32.h>

#include "common.h"
#include "data.h"
#include "weights.h"
#include "nn.h"
#include "multiply.h"

struct message {
    int sender_id;
    int mat1_address;
    int rows1, cols1;
    int mat2_address;
    int cols2;
    int result_address;
};

volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
volatile int *acceleratorIP_ISR = (int *)(ACCELERATOR_BASE_ADDRESS + 0x0c);

void my_isr_installer(void);
void my_isr(const void *arg);

void thread_accelerator(void *id, void *unused1, void *unused2);
void thread_software(void *mainIdPtr, void *myIdPtr, void *unused);

void dot_pooling(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2);
void dot(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2);
int get_digit(int num, float **digit);
void send_msg(int id, int resultAddress, int mat1Address, int mat2Address, int rows1, int cols1, int cols2);

#endif //HEADER_H