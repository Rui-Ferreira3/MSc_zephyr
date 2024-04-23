#include "multiply.h"

void multiply_mat_sw(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2)
{
    float *mat1 = (float *)mat1Address;
    float *mat2 = (float *)mat2Address;
    float *result = (float *)resultAddress;

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

void multiply_mat_hw_pool(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2)
{
    volatile int *do_matp_mem = (int *)(ACCELERATOR_BASE_ADDRESS + 0x00);
    volatile int *a = (int *)(ACCELERATOR_BASE_ADDRESS + 0x10);
	volatile int *b = (int *)(ACCELERATOR_BASE_ADDRESS + 0x18);
	volatile int *c = (int *)(ACCELERATOR_BASE_ADDRESS + 0x20);
	volatile int *rowsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x28);
	volatile int *colsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x30);
	volatile int *colsB = (int *)(ACCELERATOR_BASE_ADDRESS + 0x38);

    *a = mat1Address;
    *b = mat2Address;
    *c = resultAddress;
    *rowsA = rows1;
    *colsA = cols1;
    *colsB = cols2;

    *do_matp_mem = 1;

    while ((*do_matp_mem & 4) == 0);
}

void multiply_mat_hw(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2)
{
    volatile int *do_matp_mem = (int *)(ACCELERATOR_BASE_ADDRESS + 0x00);
    volatile int *a = (int *)(ACCELERATOR_BASE_ADDRESS + 0x10);
	volatile int *b = (int *)(ACCELERATOR_BASE_ADDRESS + 0x18);
	volatile int *c = (int *)(ACCELERATOR_BASE_ADDRESS + 0x20);
	volatile int *rowsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x28);
	volatile int *colsA = (int *)(ACCELERATOR_BASE_ADDRESS + 0x30);
	volatile int *colsB = (int *)(ACCELERATOR_BASE_ADDRESS + 0x38);

    *a = mat1Address;
    *b = mat2Address;
    *c = resultAddress;
    *rowsA = rows1;
    *colsA = cols1;
    *colsB = cols2;

    *do_matp_mem = 1;

    while ((*do_matp_mem & 4) == 0);
}