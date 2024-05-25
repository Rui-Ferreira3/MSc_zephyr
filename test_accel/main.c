/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#define DELAY 10000000

#define MAT_SIZE 4

#define N1 MAT_SIZE
#define N2 MAT_SIZE
#define N3 MAT_SIZE

#define MATA_START_ADDRESS 0x04000100
#define MATB_START_ADDRESS 0x04000200
#define MATC_START_ADDRESS 0x04000300
#define MATCS_START_ADDRESS 0x04000400

void print_mat(float *x, int colsize, int rowsize)
{
    int i, j;

    for (i=0; i<colsize; i++) {
        for (j=0; j<rowsize; j++) {
            printk("%d ", (int)x[i*rowsize+j]);
        }
        printk("\n");
    }
    printk("\n");
}

int main()
{
	volatile int *do_matp_mem = (int *)0x90000000;
    volatile int *a = (int *)0x90000010;
	volatile int *b = (int *)0x90000018;
	volatile int *c = (int *)0x90000020;
	volatile int *rowsA = (int *)0x90000028;
	volatile int *colsA = (int *)0x90000030;
	volatile int *colsB = (int *)0x90000038;

    float *matA = (float *)MATA_START_ADDRESS;
    float *matB = (float *)MATB_START_ADDRESS;
    float *matC = (float *)MATC_START_ADDRESS;
    float *matCS = (float *)MATCS_START_ADDRESS;

    *do_matp_mem = 0;

    float arrayA[N1*N2] = {
        -87,    -21,    86,     107, 
        -84,    41,     -125,   -95, 
        59,     111,    -33,    -33,
        -52,    124,    -112,   108};
    float arrayB[N2*N3] = {
        62,     84,     109,    -47, 
        -122,   -59,    -51,    25, 
        -91,    14,     -47,    -27,
        -45,    -123,   -36,    -77};

    for(int i=0; i<N1*N2; i++) matA[i] = arrayA[i];
    printk("Matrix A:\n");
    print_mat(matA, N1, N2);
    for(int i=0; i<N2*N3; i++) matB[i] = arrayB[i];
    printk("Matrix B:\n");
    print_mat(matB, N2, N3);
	for(int i=0; i<N1*N3; i++) matC[i] = 0;
	printk("Matrix C before accel:\n");
    print_mat(matC, N1, N3);

    *a = MATA_START_ADDRESS;
    *b = MATB_START_ADDRESS;
    *c = MATC_START_ADDRESS;
    *rowsA = N1;
    *colsA = N2;
    *colsB = N3;

    *do_matp_mem = 1;

    while ((*do_matp_mem & 4) == 0);

    printk("Matrix C after accel:\n");
    print_mat(matC, N1, N3);

    for (int i=0; i<N1; i++) {
        for (int j=0; j<N3; j++) {
            float val = 0;
            for (int k=0; k<N2; k++) {
                val += matA[i*N2+k]*matB[k*N3+j];
            }
            matCS[i*N3+j] = val;
        }
    }
    printk("Matrix C software:\n");
    print_mat(matCS, N1, N3);


  return 0;
}
