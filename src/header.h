#ifndef HEADER_H
#define HEADER_H

void create_mat(int address, int rows, int cols);
void print_mat(float *x, int colsize, int rowsize);
void multiply_mat(float *mat1, float *mat2, int result_address, int rows1, int cols1, int cols2);
int verify_matmul(float *mat1, float *mat2, int size);


#endif //HEADER_H