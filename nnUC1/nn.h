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

void relu(float *m, int size);
void softmax(float *m, int size);
int get_prediction(float yhat[DIGITS], int size);


#endif //NN_H