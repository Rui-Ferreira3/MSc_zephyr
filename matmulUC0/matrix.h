#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "common.h"

void create_mat(int address, int rows, int cols);
void print_mat(int address, int colsize, int rowsize);
int verify_matmul(int mat1Address, int mat2Address, int rows, int cols);


#endif //MATRIX_H