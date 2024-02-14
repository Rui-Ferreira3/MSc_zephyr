#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "header.h"
#include "defines.h"
#include "matrix.h"

int main()
{
    init_mats();

    mat_address_1 = MATI_BASE_ADDRESS;
    mat_rows_1 = MATI_ROWS;
    mat_cols_1 = MATI_COLS;
    matrix1 = matI;

    mat_address_2 = MAT1_BASE_ADDRESS;
    mat_rows_2 = MAT1_ROWS;
    mat_cols_2 = MAT1_COLS;
    matrix2 = mat1;

    software(NUM_MATMULS);
    no_multithread(NUM_MATMULS);

    return 0;
}

void no_multithread(int num_matmuls) {
    int num_errors, total_errors=0;
    uint32_t start_ms, finish_ms;

    int result_hw_base_address = MEM_BASE_ADDRESS + mat_rows_1*mat_cols_2*sizeof(float);
    float *result_sw = (float *)(result_sw_address);
    float *result_hw = (float *)(result_hw_base_address);
    printf("\nStarting %d matmuls with hardware accelerator!\n", num_matmuls);
    start_ms = k_uptime_get();
    for(int i=0; i<num_matmuls; i++) {
        multiply_mat_hw(mat_address_1, mat_address_2, result_hw_base_address, mat_rows_1, mat_cols_1, mat_cols_2);

        num_errors = verify_matmul(result_hw, result_sw, mat_rows_1, mat_cols_2);
        total_errors += num_errors;
    }
    finish_ms = k_uptime_get();

    printf("Testing done with %d errors!\nTook %u miliseconds!\n", total_errors, finish_ms-start_ms);

    print_mat(result_hw, mat_rows_1, mat_cols_2);
}

void software(int num_matmuls) {
    uint32_t start_ms, finish_ms;

    result_sw_address = MEM_BASE_ADDRESS;
    printf("Starting %d matmuls in software!\n", num_matmuls);
    start_ms = k_uptime_get();
    for(int i=0; i<num_matmuls; i++)
        multiply_mat_sw(matrix1, matrix2, result_sw_address, mat_rows_1, mat_cols_1, mat_cols_2);
    finish_ms = k_uptime_get();
    printf("Took %u miliseconds!\n", finish_ms-start_ms);
}

void init_mats() {
    for(int i=0; i<MATI_ROWS ; i++)
        for(int j=0; j<MATI_COLS; j++) {
            if(i == j) matI[i*MATI_COLS + j] = 1;
            mat1[i*MATI_COLS + j] = 1;
            mat2[i*MATI_COLS + j] = 2;
            mat3[i*MATI_COLS + j] = i;
        }
}

void create_mat(int address, int rows, int cols)
{
    float *matrix = (float *)address;

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            int random_integer = sys_rand32_get()%1000;
            matrix[i*cols+j] = (float)random_integer / 10;
        }
    }
}

void print_mat(float *matrix, int rows, int cols)
{
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            printf("%.4f\n", matrix[i*cols+j]);
        }
        printk("\n");
    }
    printk("\n\n");
}

void multiply_mat_sw(float *mat1, float *mat2, int result_address, int rows1, int cols1, int cols2)
{
    float *result = (float *)result_address;

    for (int i=0; i<rows1; i++) {
        for (int j=0; j<cols2; j++) {
            float val = 0;
            for (int k=0; k<cols1; k++) {
                val += mat1[i*cols1+k]*mat2[k*cols2+j];
            }
            result[i*cols2+j] = val;
        }
    }
}

void multiply_mat_hw(int mat1_address, int mat2_address, int result_address, int rows1, int cols1, int cols2)
{
    volatile int *do_matp_mem = (int *)(ACCELERATOR_BASE_ADDRESS + 0x00);
    volatile int *a = (int *)(ACCELERATOR_BASE_ADDRESS + 0x10);
	volatile int *b = (int *)(ACCELERATOR_BASE_ADDRESS + 0x18);
	volatile int *c = (int *)(ACCELERATOR_BASE_ADDRESS + 0x20);
	volatile int *rowsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x28);
	volatile int *colsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x30);
	volatile int *colsB = (int *)(ACCELERATOR_BASE_ADDRESS + 0x38);

    *a = mat1_address;
    *b = mat2_address;
    *c = result_address;
    *rowsA = rows1;
    *colsA = cols1;
    *colsB = cols2;

    *do_matp_mem = 1;

    while ((*do_matp_mem & 2) == 0);
}

int verify_matmul(float *mat1, float *mat2, int rows, int cols)
{
    int num_errors = 0;

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            if(mat1[i*cols+j]-mat2[i*cols+j] >= 0.01) num_errors++;
        }
    }

    return num_errors;
}