#include "header.h"

float *matI = (float *)MATI_BASE_ADDRESS;
float *mat1 = (float *)MAT1_BASE_ADDRESS;
float *mat2 = (float *)MAT2_BASE_ADDRESS;
float *mat3 = (float *)MAT3_BASE_ADDRESS;

int matrixID;

int mat_address_1;
int mat_rows_1;
int mat_cols_1;
float *matrix1;

int mat_address_2;
int mat_rows_2;
int mat_cols_2;
float *matrix2;

int main()
{
    int num_errors = 0;
    uint32_t start_sw_ms, finish_sw_ms, time_sw=0;
    uint32_t start_hw_ms, finish_hw_ms, time_hw=0;

    printf("Staring matmul UC 0!\n\n");

    printf("Defining matrices for matmul...\n\n");
    init_mats();

    printf("Performing %d matmuls...\n", NUM_MATMULS);
    for(int i=0; i<NUM_MATMULS; i++) {
        matrixID = sys_rand32_get()%NUM_MATS + 1;
        define_mats();

        start_sw_ms = k_uptime_get();

        multiply_mat_sw(mat_address_1, mat_address_2, RESULT_SW_BASE_ADDRESS, mat_rows_1, mat_cols_1, mat_cols_2);

        finish_sw_ms = k_uptime_get();
        time_sw += finish_sw_ms-start_sw_ms;

        start_hw_ms = k_uptime_get();

        multiply_mat_hw(mat_address_1, mat_address_2, RESULT_HW_BASE_ADDRESS, mat_rows_1, mat_cols_1, mat_cols_2);

        finish_hw_ms = k_uptime_get();
        time_hw += finish_hw_ms-start_hw_ms;

        num_errors += verify_matmul(RESULT_HW_BASE_ADDRESS, RESULT_SW_BASE_ADDRESS, mat_rows_1, mat_cols_2);

        // print_mat(RESULT_HW_BASE_ADDRESS, mat_cols_1, mat_rows_1);
    }

    printf("%d matmuls done with %d errors!\n", NUM_MATMULS, num_errors);
    printf("Software took %u miliseconds\n", time_sw);
    printf("Hardware took %u miliseconds\n", time_hw);

    return 0;
}

void init_mats() {
    for(int i=0; i<MATI_ROWS ; i++)
        for(int j=0; j<MATI_COLS; j++) {
            if(i == j) matI[i*MATI_COLS + j] = 1;
            else matI[i*MATI_COLS + j] = 0;
            mat1[i*MATI_COLS + j] = 1;
            mat2[i*MATI_COLS + j] = 2;
            mat3[i*MATI_COLS + j] = i;
        }
}

void define_mats() {
    mat_address_2 = MATI_BASE_ADDRESS;
    mat_rows_2 = MATI_ROWS;
    mat_cols_2 = MATI_COLS;
    matrix2 = matI;

    switch (matrixID) {
    case 1:
        mat_address_1 = MAT1_BASE_ADDRESS;
        mat_rows_1 = MAT1_ROWS;
        mat_cols_1 = MAT1_COLS;
        matrix1 = mat1;
        break;
    case 2:
        mat_address_1 = MAT2_BASE_ADDRESS;
        mat_rows_1 = MAT2_ROWS;
        mat_cols_1 = MAT2_COLS;
        matrix1 = mat2;
        break;
    case 3:
        mat_address_1 = MAT3_BASE_ADDRESS;
        mat_rows_1 = MAT3_ROWS;
        mat_cols_1 = MAT3_COLS;
        matrix1 = mat3;
        break;
    default:
        mat_address_1 = MATI_BASE_ADDRESS;
        mat_rows_1 = MATI_ROWS;
        mat_cols_1 = MATI_COLS;
        matrix1 = matI;
    }
}