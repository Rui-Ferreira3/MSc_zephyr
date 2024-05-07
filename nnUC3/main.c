#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */

#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 10

K_MSGQ_DEFINE(accel_msgq, sizeof(struct message), 10, 4);
K_MSGQ_DEFINE(reply_msgq, sizeof(struct message), 10, 4);

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_MUTEX_DEFINE(digitID_mutex);
K_MUTEX_DEFINE(completed_mutex);

static K_THREAD_STACK_ARRAY_DEFINE(stacks, NUM_THREADS+1, STACKSIZE);
static struct k_thread threads[NUM_THREADS+1];
k_tid_t thread_ids[NUM_THREADS+1];

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

    printf("*** Starting NN UC 3 with %d threads ***\n\n", NUM_THREADS);

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
    time_pool += finish_pool_ms - start_pool_ms;

    printf("Execution time: %d ms\n", time_pool);
    printf("Accuracy: %f\n", (float)accuracy_pool/DIGITS);

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
        k_thread_start(&threads[i]);
    }

    printf("\nPerforming feed forward neural network using the hardware accelerator\n");

    start_hw_ms = k_uptime_get();

    k_thread_suspend(k_current_get());

    finish_hw_ms = k_uptime_get();
    time_hw += finish_hw_ms - start_hw_ms;

    printf("Execution time: %d ms\n", time_hw);
    printf("Accuracy: %f\n", (float)accuracy_hw/DIGITS);

    printf("\n*** Exiting NN UC 3 ***\n");

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
        // printf("Received message from %d\n", msg.sender_id);

        dot_(msg.mat1_address, msg.mat2_address, msg.result_address, msg.rows1, msg.cols1, msg.cols2, msg.sender_id);

        while (k_msgq_put(&reply_msgq, &msg, K_NO_WAIT) != 0)
            k_yield();
        // printf("Sending reply to %d...\n", msg.sender_id);
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

    float *digit;
    int digity;

    int prediction;

    float *a1 = (float *) (my_a1_address);
    float *a2 = (float *) (my_a2_address);
    float *yhat = (float *) (my_yhat_address);

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
                
            multiply_mat_hw(mat1Address, (int)column, resultAddress+i*sizeof(float), rows1, cols1, 1);
            k_sem_take(&accel_sem, K_FOREVER);
        }
    }else {
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
    // printf("Thread %d sent message to central thread!\n", id);
    while (k_msgq_peek(&reply_msgq, &msg) != 0 || msg.sender_id != id) {
        k_yield();
    }
    k_msgq_get(&reply_msgq, &msg, K_FOREVER);
    // printf("Thread %d received a reply: %d!\n", id, msg.sender_id);
}
