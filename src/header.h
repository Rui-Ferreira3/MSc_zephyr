#ifndef HEADER_H
#define HEADER_H


void init_mats();
void create_mat(int address, int rows, int cols);
void print_mat(float *x, int colsize, int rowsize);
void multiply_mat_sw(float *mat1, float *mat2, int result_address, int rows1, int cols1, int cols2);
void multiply_mat_hw(int mat1_address, int mat2_address, int result_address, int rows1, int cols1, int cols2);
int verify_matmul(float *mat1, float *mat2, int rows, int cols);

void no_multithread(int num_matmuls);
void software(int num_matmuls);


#endif //HEADER_H