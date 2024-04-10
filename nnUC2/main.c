#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 10

int hwCount=0;

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(executing_sem, 0, 1);	/* starts off "not available" */

K_MUTEX_DEFINE(digitID_mutex);

static K_THREAD_STACK_ARRAY_DEFINE(stacks, NUM_THREADS, STACKSIZE);
static struct k_thread threads[NUM_THREADS];
k_tid_t thread_ids[NUM_THREADS];

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

    printf("*** Starting NN UC 2 ***\n\n");

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
        // printf("Prediction number: %d\n", i);
        // printf("Digit: %d\n", digity);
        // printf("Predicted digit: %d\n", prediction);

        if(prediction == digity) accuracy_pool++;
    }

    finish_pool_ms = k_uptime_get();
    time_pool += finish_pool_ms - start_pool_ms;

    printf("Execution time: %d ms\n", time_pool);
    printf("Accuracy: %f\n", (float)accuracy_pool/DIGITS);

    /* start all accelerator threads */
    for(int i=0; i<NUM_THREADS; i++) {
        char tname[CONFIG_THREAD_MAX_NAME_LEN];

        thread_ids[i] = k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                thread_accelerator, INT_TO_POINTER(i), NULL, NULL,
                ACCEL_THREAD_PRIO, K_USER, K_FOREVER);
        snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "thread %d", i);
		k_thread_name_set(&threads[i], tname);
        k_thread_start(&threads[i]);
    }

    printf("\nPerforming feed forward neural network using the hardware accelerator\n");

    start_hw_ms = k_uptime_get();

    k_thread_priority_set(k_current_get(), 15);

    finish_hw_ms = k_uptime_get();
    time_hw += finish_hw_ms - start_hw_ms;

    printf("Execution time: %d ms\n", time_hw);
    printf("Accuracy: %f\n", (float)accuracy_hw/DIGITS);

    printf("\n*** Exiting NN UC 2 ***\n");

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

void thread_accelerator(void *id, void *unused1, void *unused2)
{
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
    int my_id = POINTER_TO_INT(id);

    int my_a1_address = A1_BASE_ADDRESS+my_id*0x1000;
    int my_a2_address = A2_BASE_ADDRESS+my_id*0x1000;
    int my_yhat_address = YHAT_BASE_ADDRESS+my_id*0x1000;

    float *digit;
    int digity;

    int prediction;

    float *a1 = (float *) (my_a1_address);
    float *a2 = (float *) (my_a2_address);
    float *yhat = (float *) (my_yhat_address);

    for(int i=0; i<DIGITS/NUM_THREADS; i++) {
        digity = get_digit(my_id+i*NUM_THREADS, &digit);

        dot_(my_a1_address, (int)digit, (int)&W1, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        dot_(my_a2_address, my_a1_address, (int)&W2, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        dot_(my_yhat_address, my_a2_address, (int)&W3, 1, W2_COLS, W3_COLS);
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

    for(int i=0; i<cols2; i++) {
        for(int j=0; j<784; j++)
            column[j] = mat2[j*cols2+i];
            
        multiply_mat_hw_pool(mat1Address, COLUMN_BASE_ADDRESS, resultAddress+i*sizeof(float), rows1, cols1, 1);
    }

    /* enable accelerator interrupts */
    *acceleratorIP_ISR = 0x1;
    *acceleratorGIER = 0x1;
}

void dot_(int resultAddress, int mat1Address, int mat2Address, int rows1, int cols1, int cols2)
{
    float *column = (float *)COLUMN_BASE_ADDRESS;
    float *mat2 = (float *)mat2Address;

    for(int i=0; i<cols2; i++) {
        for(int j=0; j<784; j++)
            column[j] = mat2[j*cols2+i];
            
        multiply_mat_hw(mat1Address, COLUMN_BASE_ADDRESS, resultAddress+i*sizeof(float), rows1, cols1, 1);
        k_sem_take(&accel_sem, K_FOREVER);
    }
}

int get_digit(int num, float **digit)
{
    *digit = &digits[num][0];
    
    return digitsy[num];
}
