#ifndef MATRIX_H
#define MATRIX_H

struct matrix {
    float *mat;
    int rows;
    int cols;
    int address;
};

int mat_address_1;
int mat_rows_1;
int mat_cols_1;
float *matrix1;

int mat_address_2;
int mat_rows_2;
int mat_cols_2;
float *matrix2;

int result_hw_address;
int result_sw_address;

#define MATI_BASE_ADDRESS 0x02000000
#define MATI_ROWS 8
#define MATI_COLS 8
float *matI = (float *)MATI_BASE_ADDRESS;

#define MAT1_BASE_ADDRESS 0x02000100
#define MAT1_ROWS 8
#define MAT1_COLS 8
float *mat1 = (float *)MAT1_BASE_ADDRESS;

#define MAT2_BASE_ADDRESS 0x02000200
#define MAT2_ROWS 8
#define MAT2_COLS 8
float *mat2 = (float *)MAT2_BASE_ADDRESS;

#define MAT3_BASE_ADDRESS 0x02000300
#define MAT3_ROWS 8
#define MAT3_COLS 8
float *mat3 = (float *)MAT3_BASE_ADDRESS;


#endif //MATRIX_H