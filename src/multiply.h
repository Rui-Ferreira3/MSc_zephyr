#ifndef MULTIPLY_H
#define MULTIPLY_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "defines.h"

void multiply_mat_sw(float *mat1, float *mat2, int result_address, int rows1, int cols1, int cols2);
void multiply_mat_hw(int mat1_address, int mat2_address, int result_address, int rows1, int cols1, int cols2);

#endif //MULTIPLY_H