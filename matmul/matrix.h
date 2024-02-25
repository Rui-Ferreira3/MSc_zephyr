#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "defines.h"

void create_mat(int address, int rows, int cols);
void print_mat(float *x, int colsize, int rowsize);
int verify_matmul(float *mat1, float *mat2, int rows, int cols);

#endif //MATRIX_H