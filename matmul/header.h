#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "defines.h"
#include "matrix.h"
#include "multiply.h"

#define MATI_BASE_ADDRESS 0x02000000
#define MATI_ROWS 8
#define MATI_COLS 8
float *matI = (float *)MATI_BASE_ADDRESS;

#define MAT1_BASE_ADDRESS 0x02000100
#define MAT1_ROWS 8
#define MAT1_COLS 8
float *mat1 = (float *)MAT1_BASE_ADDRESS;

#define MAT2_BASE_ADDRESS 0x02000200
#define MAT2_ROWS 8
#define MAT2_COLS 8
float *mat2 = (float *)MAT2_BASE_ADDRESS;

#define MAT3_BASE_ADDRESS 0x02000300
#define MAT3_ROWS 8
#define MAT3_COLS 8
float *mat3 = (float *)MAT3_BASE_ADDRESS;

void threadAccelerator();
void threadData();

void init_mats();
void software(int num_matmuls);


#endif //HEADER_H