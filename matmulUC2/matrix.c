#include "matrix.h"

/**
 * @brief Create a random matrix
 * 
 * @param matrixAddr
 * @param rows 
 * @param cols 
 */
void create_mat(int matrixAddr, int rows, int cols)
{
    float *matrix = (float *)matrixAddr;

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            int random_integer = sys_rand32_get()%100;
            matrix[i*cols+j] = (float)random_integer / 10;
            // matrix[i*cols+j] = 1;
        }
    }
}

/**
 * @brief print matrix
 * 
 * @param matrixAddr 
 * @param rows 
 * @param cols 
 */
void print_mat(int matrixAddr, int rows, int cols)
{
    float *matrix = (float *)matrixAddr;

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            printf("%.4f\n", matrix[i*cols+j]);
        }
        printk("\n");
    }
    printk("\n\n");
}

/**
 * @brief compare mat1 and mat2
 * 
 * @param mat1Addr
 * @param mat2Addr
 * @param rows 
 * @param cols 
 * @return number of differences between mat1 and mat2
 */
int verify(int mat1Addr, int mat2Addr, int rows, int cols)
{
    int num_errors = 0;

    float *mat1 = (float *)mat1Addr;
    float *mat2 = (float *)mat2Addr;

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            if(mat1[i*cols+j]-mat2[i*cols+j] >= 0.01) num_errors++;
        }
    }

    return num_errors;
}