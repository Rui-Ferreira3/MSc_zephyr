#include "nn.h"

void dot(int resultAddress, int mat1Address, int mat2Address, int rows1, int cols1, int cols2)
{
    float *column = (float *)COLUMN_BASE_ADDRESS;
    float *mat2 = (float *)mat2Address;

    for(int i=0; i<cols2; i++) {
        for(int j=0; j<784; j++)
            column[j] = mat2[j*cols2+i];
            
        multiply_mat_hw_pool(mat1Address, COLUMN_BASE_ADDRESS, resultAddress+i*sizeof(float), rows1, cols1, 1);
    }
}

void relu(float *m, int size)
{
    for (int i=0; i<size; i++) {
        if (m[i] < 0) {
            m[i] = 0;
        }
    }
}

void softmax(float *m, int size)
{
    float max = 0;
    for (int i = 0; i<size; i++) {
        if (m[i] > max) {
            max = m[i];
        }
    }

    float sum = 0;
    for (int i=0; i<size; i++) {
        sum += exp(m[i]-max);
    }

    for (int i=0; i<size; i++) {
        m[i] = exp(m[i]-max)/sum;
    }
}

int get_prediction(float yhat[DIGITS], int size)
{
    int prediction = 0;
    float max = 0;
    for (int i=0; i<size; i++) {
        if (yhat[i] > max) {
            max = yhat[i];
            prediction = i;
        }
    }

    return prediction;
}