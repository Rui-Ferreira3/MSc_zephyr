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

volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
volatile int *acceleratorIP_ISR = (int *)(ACCELERATOR_BASE_ADDRESS + 0x0c);

void my_isr_installer(void);
void my_isr(const void *arg);

void thread_accelerator(void *mainIdPtr, void *myIdPtr, void *unused);

void dot_pooling(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2);
void dot(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2, int myId);
int get_digit(int num, float **digit);

#endif //HEADER_H