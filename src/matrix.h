#ifndef MATRIX_H
#define MATRIX_H

#include <zephyr.h>
#include <random/rand32.h>

#define MATRIX_HEAD_BASE_ADDRESS 0x0100_000

struct matGroup {
    int base_address;
    int size;

    int rows1, cols1, cols2;
    float *m1, *m2;
    float *result_hw, *result_sw;

    struct matGroup *next;
};



#endif //MATRIX_H