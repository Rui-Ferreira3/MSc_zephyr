#include "nn.h"

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