#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 10

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(thread_sem, 1, 1);	/* starts off "available" */

K_MUTEX_DEFINE(digitID_mutex);

static K_THREAD_STACK_ARRAY_DEFINE(stacks, NUM_THREADS, STACKSIZE);
static struct k_thread threads[NUM_THREADS];
k_tid_t thread_ids[NUM_THREADS];

int digitID=0;
int accuracy_hw=0;

int completed[NUM_THREADS];

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

    printf("*** Starting NN UC 2 with %d threads***\n\n", NUM_THREADS);

    printf("Installing ISR...\n");
    my_isr_installer();

    printf("\nPerforming feed forward neural network using the hardware accelerator with pooling\n");

    start_pool_ms = k_uptime_get();

    for(int i=0; i<DIGITS; i++) {
        digity = get_digit(i, &digit);

        dot((int)digit, (int)&W1, A1_BASE_ADDRESS, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        dot(A1_BASE_ADDRESS, (int)&W2, A2_BASE_ADDRESS, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        dot(A2_BASE_ADDRESS, (int)&W3, YHAT_BASE_ADDRESS, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        prediction = get_prediction(yhat, 10);

        if(prediction == digity) accuracy_pool++;
    }

    finish_pool_ms = k_uptime_get();
    time_pool = finish_pool_ms - start_pool_ms;

    printf("Execution time: %d ms\n", time_pool);
    printf("Accuracy: %f\n", (float)accuracy_pool/DIGITS);

    /* start all accelerator threads */
    for(int i=0; i<NUM_THREADS; i++) {
        char tname[CONFIG_THREAD_MAX_NAME_LEN];

        thread_ids[i] = k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                thread_accelerator, k_current_get(), INT_TO_POINTER(i), NULL,
                ACCEL_THREAD_PRIO, K_USER, K_FOREVER);
        snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "thread %d", i);
		k_thread_name_set(&threads[i], tname);
        k_thread_start(&threads[i]);
    }

    printf("\nPerforming feed forward neural network using the hardware accelerator\n");

    start_hw_ms = k_uptime_get();

    k_thread_priority_set(k_current_get(), 15);

    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;

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
    
    k_sem_give(&thread_sem);
    k_sem_give(&accel_sem);
}

/**
 * @brief thread that performs feed forward neural network using the hardware accelerator
 * 
 * @param mainIdPtr 
 * @param unused1 
 * @param unused2 
 */
void thread_accelerator(void *mainIdPtr, void *myIdPtr, void *unused)
{
    k_tid_t mainId = (k_tid_t) mainIdPtr;
    int myId = POINTER_TO_INT(myIdPtr);
	ARG_UNUSED(unused);

    int totalCompleted = 0;
    completed[myId] = 0;

    int my_a1_address = A1_BASE_ADDRESS+myId*0x1000;
    int my_a2_address = A2_BASE_ADDRESS+myId*0x1000;
    int my_yhat_address = YHAT_BASE_ADDRESS+myId*0x1000;

    float *digit;
    int digity;

    int prediction;

    float *a1 = (float *) (my_a1_address);
    float *a2 = (float *) (my_a2_address);
    float *yhat = (float *) (my_yhat_address);

    for(int i=0; i<DIGITS/NUM_THREADS; i++) {
        digity = get_digit(myId+i*NUM_THREADS, &digit);

        dot_((int)digit, (int)&W1, my_a1_address, 1, DIGIT_SIZE, W1_COLS, myId);
        relu(a1, W1_COLS);

        dot_(my_a1_address, (int)&W2, my_a2_address, 1, W1_COLS, W2_COLS, myId);
        relu(a2, W2_COLS);

        dot_(my_a2_address, (int)&W3, my_yhat_address, 1, W2_COLS, W3_COLS, myId);
        softmax(yhat, W3_COLS);

        prediction = get_prediction(yhat, 10);

        if(prediction == digity) accuracy_hw++;
    }

    completed[myId] = 1;

    k_mutex_lock(&completed_mutex, K_FOREVER);
    for(int i=0; i<NUM_THREADS; i++)
        totalCompleted += completed[i];
    k_mutex_unlock(&completed_mutex);

    if(totalCompleted == NUM_THREADS)
        k_thread_resume(mainId);
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
void dot(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2)
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
void dot_(int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2, int myId)
{
    float *column = (float *)COLUMN_BASE_ADDRESS+myId*MAX_MATRIX_SIZE*sizeof(float);
    float *mat2 = (float *)mat2Address;

    if(cols1*cols2 > MAX_MATRIX_SIZE) {
        for(int i=0; i<cols2; i++) {
            for(int j=0; j<cols1; j++)
                column[j] = mat2[j*cols2+i];
                
            k_sem_take(&thread_sem, K_FOREVER);
            multiply_mat_hw(mat1Address, (int)column, resultAddress+i*sizeof(float), rows1, cols1, 1);
            k_sem_take(&accel_sem, K_FOREVER);
        }
    }else {
        k_sem_take(&thread_sem, K_FOREVER);
        multiply_mat_hw(mat1Address, mat2Address, resultAddress, rows1, cols1, cols2);
        k_sem_take(&accel_sem, K_FOREVER);
    }
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
