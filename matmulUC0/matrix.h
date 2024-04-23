#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "common.h"
#include "multiply.h"

struct matmul {
    float *mat1, *mat2;
    int mat1Rows, mat1Cols, mat2Cols;
    float *resultHW, *resultSW;
    struct matmul *next;
};


void create_mat(int matrixAddr, int rows, int cols);
void print_mat(int matrixAddr, int rows, int cols);
int verify(int mat1Addr, int mat2Addr, int rows, int cols);

#endif //MATRIX_H