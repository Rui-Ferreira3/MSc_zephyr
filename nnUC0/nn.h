#ifndef NN_H
#define NN_H

#include <stdio.h>
#include <math.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "common.h"
#include "multiply.h"

// void dot_sw(float *m1, float *m2, float **result, int rows, int cols);
// void dot_hw_pooling(float *m1, float *m2, float **result, int rows, int cols);

void dot(int resultAddress, int mat1Address, int mat2Address, int rows1, int cols1, int cols2);
void relu(float *m, int size);
void softmax(float *m, int size);

int get_prediction(float yhat[DIGITS], int size);


#endif //NN_H