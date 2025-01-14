#include "matrix.h"

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