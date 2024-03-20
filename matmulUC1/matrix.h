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


/**
 * @brief initialize new matmul node
 * 
 * @param node 
 * @param rows1 
 * @param cols1 
 * @param cols2 
 * @return -1 if any error occured allocating memory; 0 otherwise
 */
int init_matmul(struct matmul *node, int rows1, int cols1, int cols2);

/**
 * @brief fill the queue with num_nodes operations
 * 
 * @param head 
 * @param num_matmuls 
 */
void fill_queue(struct matmul *head, int num_matmuls);

/**
 * @brief fill both input matrixes in node
 * 
 * @param node 
 */
void fill_matmul(struct matmul *node);

/**
 * @brief Create a random matrix
 * 
 * @param matrix 
 * @param rows 
 * @param cols 
 */
void create_mat(float *matrix, int rows, int cols);

/**
 * @brief perform num_matmuls operations
 * 
 * @param head 
 * @param num_matmuls 
 * @return time in miliseconds
 */
int software_matmul(struct matmul *head);

int pooling_matmul(struct matmul *head);

/**
 * @brief multiply matrixes in node
 * 
 * @param calculatingNode 
 */
void hardware_matmul(struct matmul *node);

/**
 * @brief verify all matmuls in queue
 * 
 * @param head 
 * @param num_matmuls 
 * @return int 
 */
int verify_queue(struct matmul *head, int num_matmuls);

/**
 * @brief compare mat1 and mat2
 * 
 * @param mat1 
 * @param mat2 
 * @param rows 
 * @param cols 
 * @return number of differences between mat1 and mat2
 */
int verify_matmul(struct matmul *node);

/**
 * @brief print matrix
 * 
 * @param matrix 
 * @param rows 
 * @param cols 
 */
void print_mat(float *matrix, int rows, int cols);

/**
 * @brief push new node at the end of the queue
 * 
 * @param head 
 * @param newNode 
 */
void push_matmul(struct matmul *head, struct matmul *newNode);

/**
 * @brief push new node at beggining of queue
 * 
 * @param head 
 * @param newNode 
 */
void push2_matmul(struct matmul *head, struct matmul *newNode);

/**
 * @brief remove node from the end of the queue
 * 
 * @param head 
 * @param node 
 */
void pop_matmul(struct matmul *head, struct matmul **node);

/**
 * @brief remove first node from the queue
 * 
 * @param head 
 * @param node 
 */
void pop2_matmul(struct matmul *head, struct matmul **node);

int reset_queue(struct matmul *head, struct matmul *completedHead);

/**
 * @brief free memory used buy a single node
 * 
 * @param node 
 */
void free_matmul(struct matmul *node);

/**
 * @brief free full queue
 * 
 * @param head 
 */
void free_queue(struct matmul *head);

#endif //MATRIX_H