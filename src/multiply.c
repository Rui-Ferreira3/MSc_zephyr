#include "multiply.h"

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