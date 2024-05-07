#include "header.h"

int main()
{
    uint32_t start_sw_ms, finish_sw_ms, time_sw=0;
    uint32_t start_hw_ms, finish_hw_ms, time_hw=0;

    int accuracy_sw=0, accuracy_hw=0;
    
    float *a1 = (float *) (A1_BASE_ADDRESS);
    float *a2 = (float *) (A2_BASE_ADDRESS);
    float *yhat = (float *) (YHAT_BASE_ADDRESS);

    int digity, prediction;
    float *digit;

    printf("*** Starting NN UC 0 ***\n\n");

    printf("Performing feed forward neural network using software only\n");
    for(int i=0; i<DIGITS; i++) {
        digity = get_digit(i, &digit);

        start_sw_ms = k_uptime_get();

        multiply_mat_sw((int)digit, (int)&W1, A1_BASE_ADDRESS, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        multiply_mat_sw(A1_BASE_ADDRESS, (int)&W2, A2_BASE_ADDRESS, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        multiply_mat_sw(A2_BASE_ADDRESS, (int)&W3, YHAT_BASE_ADDRESS, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        finish_sw_ms = k_uptime_get();
        time_sw += finish_sw_ms - start_sw_ms;
        
        prediction = get_prediction(yhat, 10);

        if(prediction == digity) accuracy_sw++;
    }
    printf("Software execution time: %d ms\n", time_sw);
    printf("Accuracy: %f\n", (float)accuracy_sw/DIGITS);

    printf("\nPerforming feed forward neural network using hardware\n");
    for(int i=0; i<DIGITS; i++) {
        digity = get_digit(i, &digit);

        start_hw_ms = k_uptime_get();

        dot((int)digit, (int)&W1, A1_BASE_ADDRESS, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        dot(A1_BASE_ADDRESS, (int)&W2, A2_BASE_ADDRESS, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        dot(A2_BASE_ADDRESS, (int)&W3, YHAT_BASE_ADDRESS, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        finish_hw_ms = k_uptime_get();
        time_hw += finish_hw_ms - start_hw_ms;

        prediction = get_prediction(yhat, 10);

        if(prediction == digity) accuracy_hw++;
    }
    printf("Hardware execution time: %d ms\n", time_hw);
    printf("Accuracy: %f\n", (float)accuracy_hw/DIGITS);


    printf("\n*** Exiting NN UC 0 ***\n");

    return 0;
}

/**
 * @brief dot product of two matrices
 * @brief if the size of the matrices is greater than MAX_MATRIX_SIZE,
 * @brief the function will split the second matrix into columns and multiply each column with the first matrix
 * 
 * @param mat1Address 
 * @param mat2Address 
 * @param resultAddress 
 * @param rows1 
 * @param cols1 
 * @param cols2 
 */
void dot(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2)
{
    float *column = (float *)COLUMN_BASE_ADDRESS;
    float *mat2 = (float *)mat2Address;

    if(cols1*cols2 > MAX_MATRIX_SIZE) {
        for(int i=0; i<cols2; i++) {
            for(int j=0; j<cols1; j++)
                column[j] = mat2[j*cols2+i];
                
            multiply_mat_hw(mat1Address, COLUMN_BASE_ADDRESS, resultAddress+i*sizeof(float), rows1, cols1, 1);
        }
    }else
        multiply_mat_hw(mat1Address, mat2Address, resultAddress, rows1, cols1, cols2);
}

/**
 * @brief Get the digit object
 * 
 * @param num 
 * @param digit 
 * @return int 
 */
int get_digit(int num, float **digit)
{
    *digit = &digits[num][0];
    
    return digitsy[num];
}