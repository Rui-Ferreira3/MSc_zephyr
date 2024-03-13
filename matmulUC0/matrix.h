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


void create_mat(float *matrix, int rows, int cols);
void print_mat(float *matrix, int rows, int cols);
int verify_matmul(float *mat1, float *mat2, int rows, int cols);

int init_matmul(struct matmul *node, int rows1, int cols1, int cols2);
void fill_matmul(struct matmul *node);
void push_matmul(struct matmul *head, struct matmul *newNode);
void pop_matmul(struct matmul *head, struct matmul **node);
void pop2_matmul(struct matmul *head, struct matmul **node);
void save_matmul(struct matmul *node);
void perform_matmul(struct matmul *calculatingNode);
void free_matmul(struct matmul *node);
void free_queue(struct matmul *head);

#endif //MATRIX_H