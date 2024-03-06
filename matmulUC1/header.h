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

struct matmul *head, *calculatingNode;

void my_isr_installer(void);
void my_isr(const void *arg);
void thread_accelerator();
void thread_software();
void init_mats();
void define_mats();


#endif //HEADER_H