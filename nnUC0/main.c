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

        multiply_mat_sw(a1, digit, W1, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        multiply_mat_sw(a2, a1, W2, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        multiply_mat_sw(yhat, a2, W3, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        finish_sw_ms = k_uptime_get();
        time_sw += finish_sw_ms - start_sw_ms;
        
        prediction = get_prediction(yhat, 10);
        printf("Digit: %d\n", digity);
        printf("Predicted digit: %d\n", prediction);

        if(prediction == digity) accuracy_sw++;
    }
    printf("Software execution time: %d ms\n", time_sw);
    printf("Accuracy: %f\n", (float)accuracy_sw/DIGITS);

    printf("\nPerforming feed forward neural network using hardware\n");
    for(int i=0; i<DIGITS; i++) {
        printf("Prediction number: %d\n", i);
        digity = get_digit(i, &digit);

        start_hw_ms = k_uptime_get();

        dot(A1_BASE_ADDRESS, (int)digit, (int)&W1, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        dot(A2_BASE_ADDRESS, A1_BASE_ADDRESS, (int)&W2, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        dot(YHAT_BASE_ADDRESS, A2_BASE_ADDRESS, (int)&W3, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        finish_hw_ms = k_uptime_get();
        time_hw += finish_hw_ms - start_hw_ms;

        prediction = get_prediction(yhat, 10);
        printf("Digit: %d\n", digity);
        printf("Predicted digit: %d\n", prediction);

        if(prediction == digity) accuracy_hw++;
    }
    printf("Hardware execution time: %d ms\n", time_hw);
    printf("Accuracy: %f\n", (float)accuracy_hw/DIGITS);


    printf("\n*** Exiting NN UC 0 ***\n");

    return 0;
}

int get_digit(int num, float **digit)
{
    *digit = &digits[num][0];
    
    return digitsy[num];
}