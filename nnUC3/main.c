#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 15       /* device uses interrupt priority 15 */
#define ACCEL_THREAD_PRIO 8
#define SW_THREAD_PRIO 10

K_MSGQ_DEFINE(accel_msgq, sizeof(struct message), 8, 4);
K_MSGQ_DEFINE(reply_msgq, sizeof(struct message), 8, 4);

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_MUTEX_DEFINE(completed_mutex);

static K_THREAD_STACK_ARRAY_DEFINE(stacks, NUM_THREADS+1, STACKSIZE);
static struct k_thread threads[NUM_THREADS+1];
k_tid_t thread_ids[NUM_THREADS+1];
int accuracy_hw=0;

int completed[NUM_THREADS];

uint32_t thread_ms, time_thread=0;
uint32_t sw_ms, time_sw=0;

int main()
{
    printf("*** Starting NN UC 3 with %d threads ***\n", NUM_THREADS);

    uint32_t start_hw_ms, finish_hw_ms, time_hw;

    printf("Installing ISR...\n");
    my_isr_installer();

    /* start all accelerator threads */
    for(int i=0; i<NUM_THREADS+1; i++) {
        char tname[CONFIG_THREAD_MAX_NAME_LEN];

        if(i == NUM_THREADS) {
            k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                    thread_accelerator, INT_TO_POINTER(i), NULL, NULL,
                    ACCEL_THREAD_PRIO, K_USER, K_FOREVER);
            snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "accelerator");
        }else {
            thread_ids[i] = k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                    thread_software, k_current_get(), INT_TO_POINTER(i), NULL,
                    SW_THREAD_PRIO, K_USER, K_FOREVER);
            snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "thread %d", i);
        }

		k_thread_name_set(&threads[i], tname);
    }

    printf("\nPerforming feed forward neural network using the hardware accelerator\n");

    k_msleep(10000);

    for(int i=0; i<NUM_THREADS+1; i++)
        k_thread_start(&threads[i]);

    start_hw_ms = k_uptime_get();

    k_thread_suspend(k_current_get());

    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;

    printf("Execution time: %d ms\n", time_hw);
    printf("Accuracy: %f\n", (float)accuracy_hw/DIGITS);

    printf("\n*** Exiting NN UC 3 ***\n");

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
 * @brief accelerator thread
 * @brief gets message from message queue
 * @brief performs matrix multiplication
 * @brief sends reply to software thread
 * 
 * @param id 
 * @param unused1 
 * @param unused2 
 */
void thread_accelerator(void *id, void *unused1, void *unused2)
{
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    struct message msg;

    while(1) {
        k_msgq_get(&accel_msgq, &msg, K_FOREVER);

        dot(msg.mat1_address, msg.mat2_address, msg.result_address, msg.rows1, msg.cols1, msg.cols2);

        k_msgq_put(&reply_msgq, &msg, K_FOREVER);
    }
}

/**
 * @brief software thread
 * @brief sends message to accelerator thread
 * @brief wakes main thread when all threads are complete
 * 
 * @param mainIdPtr 
 * @param myIdPtr 
 * @param unused 
 */
void thread_software(void *mainIdPtr, void *myIdPtr, void *unused)
{
    k_tid_t mainId = (k_tid_t) mainIdPtr;
    int myId = POINTER_TO_INT(myIdPtr);
	ARG_UNUSED(unused);

    int totalCompleted = 0;
    completed[myId] = 0;

    int addressOffset = A1_SIZE+A2_SIZE+YHAT_SIZE;

    int my_a1_address = A1_BASE_ADDRESS+myId*addressOffset;
    int my_a2_address = A2_BASE_ADDRESS+myId*addressOffset;
    int my_yhat_address = YHAT_BASE_ADDRESS+myId*addressOffset;

    float *a1 = (float *) (my_a1_address);
    float *a2 = (float *) (my_a2_address);
    float *yhat = (float *) (my_yhat_address);

    float *digit;
    int digity;

    int prediction;

    for(int i=0; i<DIGITS/NUM_THREADS; i++) {
        digity = get_digit(myId+i*NUM_THREADS, &digit);

        send_msg(myId, my_a1_address, (int)digit, (int)&W1, 1, DIGIT_SIZE, W1_COLS);
        relu(a1, W1_COLS);

        send_msg(myId, my_a2_address, my_a1_address, (int)&W2, 1, W1_COLS, W2_COLS);
        relu(a2, W2_COLS);

        send_msg(myId, my_yhat_address, my_a2_address, (int)&W3, 1, W2_COLS, W3_COLS);
        softmax(yhat, W3_COLS);

        prediction = get_prediction(yhat, 10);

        if(prediction == digity) accuracy_hw++;
    }

    k_mutex_lock(&completed_mutex, K_FOREVER);
    completed[myId] = 1;
    for(int i=0; i<NUM_THREADS; i++)
        totalCompleted += completed[i];
    k_mutex_unlock(&completed_mutex);

    if(totalCompleted == NUM_THREADS)
        k_thread_resume(mainId);
}

/**
 * @brief sends message to accelerator thread
 * @brief waits for reply
 * 
 * @param id 
 * @param mat1Address 
 * @param mat2Address 
 * @param resultAddress 
 * @param rows1 
 * @param cols1 
 * @param cols2 
 */
void send_msg(int id, int resultAddress, int mat1Address, int mat2Address, int rows1, int cols1, int cols2)
{
    struct message msg;

    msg.sender_id = id;
    msg.result_address = resultAddress;
    msg.mat1_address = mat1Address;
    msg.mat2_address = mat2Address;
    msg.rows1 = rows1;
    msg.cols1 = cols1;
    msg.cols2 = cols2;

    while (k_msgq_put(&accel_msgq, &msg, K_NO_WAIT) != 0) {
        k_yield();
    }

    while (k_msgq_peek(&reply_msgq, &msg) != 0 || msg.sender_id != id) {
        k_yield();
    }
    k_msgq_get(&reply_msgq, &msg, K_FOREVER);
}