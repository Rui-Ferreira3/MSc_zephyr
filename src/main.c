#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "header.h"
#include "defines.h"

int main()
{
    int rows1=16, cols1=8, cols2=10;
    int mat1_base_address, mat2_base_address, result_hw_base_address, result_sw_base_address;

    mat1_base_address = MAT_BASE_ADDRESS;
    float *mat1 = (float *)mat1_base_address;

    mat2_base_address = mat1_base_address + rows1*cols1*sizeof(float);
    float *mat2 = (float *)mat2_base_address;

    result_hw_base_address = mat2_base_address + cols1*cols2*sizeof(float);
    float *result_hw = (float *)(result_hw_base_address);

    result_sw_base_address = result_hw_base_address + rows1*cols2*sizeof(float);
    float *result_sw = (float *)(result_sw_base_address);

    volatile int *do_matp_mem = (int *)(ACCELERATOR_BASE_ADDRESS + 0x00);
    volatile int *a = (int *)(ACCELERATOR_BASE_ADDRESS + 0x10);
	volatile int *b = (int *)(ACCELERATOR_BASE_ADDRESS + 0x18);
	volatile int *c = (int *)(ACCELERATOR_BASE_ADDRESS + 0x20);
	volatile int *rowsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x28);
	volatile int *colsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x30);
	volatile int *colsB = (int *)(ACCELERATOR_BASE_ADDRESS + 0x38);

    create_mat(mat1_base_address, rows1, cols1);
    printk("Printing Matrix 1!\n");
    print_mat(mat1, rows1, cols1);

    create_mat(mat2_base_address, cols1, cols2);
    printk("Printing Matrix 2!\n");
    print_mat(mat2, cols1, cols2);

    multiply_mat(mat1, mat2, result_sw_base_address, rows1, cols1, cols2);
    printk("Printing Software Result!\n");
    print_mat(result_sw, rows1, cols2);

    *a = mat1_base_address;
    *b = mat2_base_address;
    *c = result_hw_base_address;
    *rowsA = rows1;
    *colsA = cols1;
    *colsB = cols2;

    *do_matp_mem = 1;

    while ((*do_matp_mem & 2) == 0);

    printk("Printing Hardware Result!\n");
    print_mat(result_hw, rows1, cols2);

    int num_errors = verify_matmul(result_hw, result_sw, rows1*cols2);
    printk("Accelerator done with %d errors!\n", num_errors);

    return 0;
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
            printk("%d\t", (int)matrix[i*cols+j]);
        }
        printk("\n");
    }
    printk("\n");
}

void multiply_mat(float *mat1, float *mat2, int result_address, int rows1, int cols1, int cols2)
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

int verify_matmul(float *mat1, float *mat2, int size)
{
    int num_errors = 0;

    for(int i=0; i<size; i++)
        if(mat1[i]-mat2[1] >= 0.1) num_errors++;

    return num_errors;
}