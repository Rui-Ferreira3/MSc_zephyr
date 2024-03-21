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

struct message {
    int sender_id;
    struct matmul *node;
};

struct matmul *head, *completedHead;

volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
volatile int *acceleratorIP_ISR = (int *)(ACCELERATOR_BASE_ADDRESS + 0x0c);

void isr_installer(void);
void accel_isr(const void *arg);
void thread_reset(void *id, void *unused1, void *unused2);
void thread_software(void *id, void *unused1, void *unused2);
void thread_accelerator(void *id, void *unused1, void *unused2);

#endif //HEADER_H