#ifndef MULTIPLY_H
#define MULTIPLY_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "common.h"

void multiply_mat_sw(float *result, float *mat1, float *mat2, int rows1, int cols1, int cols2);
void multiply_mat_hw(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2);

#endif //MULTIPLY_H