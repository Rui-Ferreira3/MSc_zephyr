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

int completed[NUM_THREADS];

int main()
{
    printf("*** Starting matrix multiplication UC 3 with %d threads ***\n", NUM_THREADS);
    
    uint32_t start_hw_ms, finish_hw_ms, time_hw;
    int addressOffset = MATRIX1_SIZE + MATRIX2_SIZE + RESULT_POOL_SIZE + RESULT_HW_SIZE;

    printf("\nInstalling ISR...\n");
    my_isr_installer();

    /* initialize the queue */
    printf("\nSaving %d matrix pairs to memory...\n", NUM_MULTIPLICATIONS);
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        create_mat(mat1Addr, MAT1ROWS, MAT1COLS);

        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        create_mat(mat2Addr, MAT1ROWS, MAT1COLS);
    }
    printf("%d saved to memory!\n", NUM_MULTIPLICATIONS);

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

    printf("\nPerforming matrix multiplication using the hardware accelerator...\n");

    k_msleep(10000);

    for(int i=0; i<NUM_THREADS+1; i++)
        k_thread_start(&threads[i]);

    start_hw_ms = k_uptime_get();

    k_thread_suspend(k_current_get());

    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;

    printf("\nExecution time: %d ms\n", time_hw);

    printf("\n*** Exiting matrix multiplication UC 3 ***\n");

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

        multiply_mat_hw(msg.mat1_address, msg.mat2_address, msg.result_address, msg.rows1, msg.cols1, msg.cols2);
        k_sem_take(&accel_sem, K_FOREVER);

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

    int addressOffset = MATRIX1_SIZE + MATRIX2_SIZE + RESULT_POOL_SIZE + RESULT_HW_SIZE;

    int mat1Addr = MEMORY_BASE_ADDRESS + myId*addressOffset;
    int mat2Addr = mat1Addr + MATRIX1_SIZE;
    int resultAddr = mat2Addr + MATRIX2_SIZE + RESULT_POOL_SIZE;

    for(int i=myId; i<NUM_MULTIPLICATIONS; i+=NUM_THREADS) {
        send_msg(myId, mat1Addr, mat2Addr, resultAddr, MAT1ROWS, MAT1COLS, MAT2COLS);
    }

    k_mutex_lock(&completed_mutex, K_FOREVER);
    completed[myId] = 1;
    k_mutex_unlock(&completed_mutex);

    k_mutex_lock(&completed_mutex, K_FOREVER);
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
void send_msg(int id, int mat1Address, int mat2Address, int resultAddress, int rows1, int cols1, int cols2)
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