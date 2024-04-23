#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 10

int hwCount=0;

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */
// K_SEM_DEFINE(executing_sem, 0, 1);	/* starts off "not available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

// K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
// static struct k_thread threadB_data;

K_MUTEX_DEFINE(digitID_mutex);

int digitID=0;
int accuracy_hw=0;

int main()
{
    uint32_t start_pool_ms, finish_pool_ms, time_pool=0;
    uint32_t start_hw_ms, finish_hw_ms, time_hw=0;
    
    float *digit;
    int digity;

    int accuracy_pool=0;
    int prediction;
    
    float *a1 = (float *) (A1_BASE_ADDRESS);
    float *a2 = (float *) (A2_BASE_ADDRESS);
    float *yhat = (float *) (YHAT_BASE_ADDRESS);

    printf("*** Starting NN UC 1 ***\n\n");

    printf("Installing ISR...\n");
    my_isr_installer();

    printf("\nPerforming feed forward neural network using the hardware accelerator with pooling\n");
    
    start_pool_ms = k_uptime_get();

    for(int i=0; i<DIGITS; i++) {
        digity = get_digit(i, &digit);

        dot(A1_BASE_ADDRESS, (int)digit, (int)&W1, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        dot(A2_BASE_ADDRESS, A1_BASE_ADDRESS, (int)&W2, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        dot(YHAT_BASE_ADDRESS, A2_BASE_ADDRESS, (int)&W3, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        prediction = get_prediction(yhat, 10);
        // printf("Digit: %d\n", digity);
        // printf("Predicted digit: %d\n", prediction);

        if(prediction == digity) accuracy_pool++;
    }

    finish_pool_ms = k_uptime_get();
    time_pool = finish_pool_ms - start_pool_ms;

    printf("Execution time: %d ms\n", time_pool);
    printf("Accuracy: %f\n", (float)accuracy_pool/DIGITS);

    /* starting software and accelerator threads*/
    k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, NULL, NULL, NULL,
        ACCEL_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");
    k_thread_start(&threadA_data);

    // k_thread_create(&threadB_data, threadB_stack_area,
    //     K_THREAD_STACK_SIZEOF(threadB_stack_area),
    //     thread_software, NULL, NULL, NULL,
    //     SW_THREAD_PRIO, 0, K_FOREVER);
    // k_thread_name_set(&threadB_data, "thread_b");
    // k_thread_start(&threadB_data);
    printf("\nPerforming feed forward neural network using the hardware accelerator\n");

    start_hw_ms = k_uptime_get();

    k_thread_priority_set(k_current_get(), 15);

    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;

    printf("Execution time: %d ms\n", time_hw);
    printf("Accuracy: %f\n", (float)accuracy_hw/DIGITS);
    printf("HW Count: %d\n", hwCount);

    printf("\n*** Exiting NN UC 1 ***\n");

    return 0;
}

/**
 * @brief installs accelerator isr
 * 
 */
void my_isr_installer(void) {
    *acceleratorGIER = 0x1;
    *acceleratorIP_IER = 0x1;
    ARCH_IRQ_CONNECT(ACCEL_IRQ, ACCEL_PRIO, my_isr, NULL, 0);
    arch_irq_enable(ACCEL_IRQ);
}

/**
 * @brief accelerator isr
 * @brief disables accelerator interrupt
 * @brief flags that accelerator is ready
 * @brief gives the semaphore for accelerator thread to procede
 * 
 * @param arg 
 */
void my_isr(const void *arg) {
    *acceleratorIP_ISR = 0x1;
    
    k_sem_give(&accel_sem);
}

void thread_software()
{
    float *digit;
    int digity;

    int prediction;

    float *a1 = (float *) (A1_BASE_ADDRESS+0x1000);
    float *a2 = (float *) (A2_BASE_ADDRESS+0x1000);
    float *yhat = (float *) (YHAT_BASE_ADDRESS+0x1000);

    while (digitID < DIGITS) {
        k_mutex_lock(&digitID_mutex, K_FOREVER);
        if(digitID < DIGITS) {
            digity = get_digit(digitID, &digit);
            digitID++;
        }else {
            k_mutex_unlock(&digitID_mutex);
            break;
        }
        k_mutex_unlock(&digitID_mutex);

        multiply_mat_sw(a1, digit, W1, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        multiply_mat_sw(a2, a1, W2, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        multiply_mat_sw(yhat, a2, W3, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        prediction = get_prediction(yhat, 10);
        // printf("Digit: %d\n", digity);
        // printf("Predicted digit: %d\n", prediction);

        if(prediction == digity) accuracy_hw++;
    }
}

void thread_accelerator()
{
    float *digit;
    int digity;

    int prediction;

    float *a1 = (float *) (A1_BASE_ADDRESS);
    float *a2 = (float *) (A2_BASE_ADDRESS);
    float *yhat = (float *) (YHAT_BASE_ADDRESS);

    while (digitID < DIGITS) {
        k_mutex_lock(&digitID_mutex, K_FOREVER);
        if(digitID < DIGITS) {
            digity = get_digit(digitID, &digit);
            digitID++;
            hwCount++;
        }else {
            k_mutex_unlock(&digitID_mutex);
            break;
        }
        k_mutex_unlock(&digitID_mutex);

        dot_(A1_BASE_ADDRESS, (int)digit, (int)&W1, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        dot_(A2_BASE_ADDRESS, A1_BASE_ADDRESS, (int)&W2, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        dot_(YHAT_BASE_ADDRESS, A2_BASE_ADDRESS, (int)&W3, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        prediction = get_prediction(yhat, 10);
        // printf("Digit: %d\n", digity);
        // printf("Predicted digit: %d\n", prediction);

        if(prediction == digity) accuracy_hw++;
    }
}

void dot(int resultAddress, int mat1Address, int mat2Address, int rows1, int cols1, int cols2)
{
    float *column = (float *)COLUMN_BASE_ADDRESS;
    float *mat2 = (float *)mat2Address;

    volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
    volatile int *acceleratorIP_ISR = (int *)(ACCELERATOR_BASE_ADDRESS + 0x0c);

    /* disable accelerator interrupts */
    *acceleratorGIER = 0x0;

    if(cols1*cols2 > MAX_MATRIX_SIZE) {
        for(int i=0; i<cols2; i++) {
            for(int j=0; j<cols1; j++)
                column[j] = mat2[j*cols2+i];
                
            multiply_mat_hw_pool(mat1Address, COLUMN_BASE_ADDRESS, resultAddress+i*sizeof(float), rows1, cols1, 1);
        }
    }else
        multiply_mat_hw_pool(mat1Address, mat2Address, resultAddress, rows1, cols1, cols2);

    /* enable accelerator interrupts */
    *acceleratorIP_ISR = 0x1;
    *acceleratorGIER = 0x1;
}

void dot_(int resultAddress, int mat1Address, int mat2Address, int rows1, int cols1, int cols2)
{
    float *column = (float *)COLUMN_BASE_ADDRESS;
    float *mat2 = (float *)mat2Address;

    if(cols1*cols2 > MAX_MATRIX_SIZE) {
        for(int i=0; i<cols2; i++) {
            for(int j=0; j<cols1; j++)
                column[j] = mat2[j*cols2+i];
                
            multiply_mat_hw(mat1Address, COLUMN_BASE_ADDRESS, resultAddress+i*sizeof(float), rows1, cols1, 1);
            k_sem_take(&accel_sem, K_FOREVER);
        }
    }else {
        multiply_mat_hw(mat1Address, mat2Address, resultAddress, rows1, cols1, cols2);
        k_sem_take(&accel_sem, K_FOREVER);
    }
}

int get_digit(int num, float **digit)
{
    *digit = &digits[num][0];
    
    return digitsy[num];
}