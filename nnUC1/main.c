#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 15       /* device uses interrupt priority 15 */
#define ACCEL_THREAD_PRIO 10

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

int accuracy_hw=0;

int main()
{
    printf("*** Starting NN UC 1 ***\n");

    uint32_t start_hw_ms, finish_hw_ms, time_hw;

    printf("\nInstalling ISR...\n");
    my_isr_installer();

    /* starting accelerator thread*/
    k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, k_current_get(), NULL, NULL,
        ACCEL_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");
    
    printf("\nPerforming feed forward neural network using the hardware accelerator\n");

    k_msleep(10000);
    
    k_thread_start(&threadA_data);

    start_hw_ms = k_uptime_get();

    k_thread_suspend(k_current_get());

    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;

    printf("Execution time: %d ms\n", time_hw);
    printf("Accuracy: %f\n", (float)accuracy_hw/DIGITS);

    printf("\n*** Exiting NN UC 1 ***\n");

    return 0;
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

/**
 * @brief dot product of two matrices using the hardware accelerator with pooling
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
void dot_pooling(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2)
{
    float *column = (float *)COLUMN_BASE_ADDRESS;
    float *mat2 = (float *)mat2Address;

    if(cols1*cols2 > MAX_MATRIX_SIZE) {
        for(int i=0; i<cols2; i++) {
            for(int j=0; j<cols1; j++)
                column[j] = mat2[j*cols2+i];
                
            multiply_mat_hw_pool(mat1Address, COLUMN_BASE_ADDRESS, resultAddress+i*sizeof(float), rows1, cols1, 1);
        }
    }else
        multiply_mat_hw_pool(mat1Address, mat2Address, resultAddress, rows1, cols1, cols2);
}

/**
 * @brief dot product of two matrices using the hardware accelerator with interrupts
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
            k_sem_take(&accel_sem, K_FOREVER);
        }
    }else {
        multiply_mat_hw(mat1Address, mat2Address, resultAddress, rows1, cols1, cols2);
        k_sem_take(&accel_sem, K_FOREVER);
    }
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

/**
 * @brief thread that performs feed forward neural network using the hardware accelerator
 * 
 * @param mainIdPtr 
 * @param unused1 
 * @param unused2 
 */
void thread_accelerator(void *mainIdPtr, void *unused1, void *unused2)
{
    k_tid_t mainId = (k_tid_t) mainIdPtr;
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    float *digit;
    int digity;

    float *a1 = (float *) (A1_BASE_ADDRESS);
    float *a2 = (float *) (A2_BASE_ADDRESS);
    float *yhat = (float *) (YHAT_BASE_ADDRESS);

    int prediction;

    for(int i=0; i<DIGITS; i++) {
        digity = get_digit(i, &digit);

        dot((int)digit, (int)&W1, A1_BASE_ADDRESS, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        dot(A1_BASE_ADDRESS, (int)&W2, A2_BASE_ADDRESS, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        dot(A2_BASE_ADDRESS, (int)&W3, YHAT_BASE_ADDRESS, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        prediction = get_prediction(yhat, 10);

        if(prediction == digity) accuracy_hw++;
    }

    k_thread_resume(mainId);
}