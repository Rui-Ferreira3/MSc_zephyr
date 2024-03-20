#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "common.h"
#include "matrix.h"
#include "multiply.h"

struct matmul *head, *completedHead;

volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
volatile int *acceleratorIP_ISR = (int *)(ACCELERATOR_BASE_ADDRESS + 0x0c);

void my_isr_installer(void);
void my_isr(const void *arg);
void thread_reset();
void thread_accelerator(void *id, void *unused1, void *unused2);

#endif //HEADER_H