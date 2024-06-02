#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 15       /* device uses interrupt priority 15 */
#define ACCEL_THREAD_PRIO 10

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

int main()
{
    printf("*** Starting matrix multiplication UC 1 ***\n");

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

    /* starting accelerator thread */
    k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, k_current_get(), NULL, NULL,
        ACCEL_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");

    printf("\nPerforming matrix multiplication using the hardware accelerator...\n");

    k_msleep(10000);

    k_thread_start(&threadA_data);

    start_hw_ms = k_uptime_get();

    k_thread_suspend(k_current_get());

    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;

    printf("Execution time: %d ms\n", time_hw);

    printf("\n*** Exiting matrix multiplication UC 1 ***\n");

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
 * @brief Single thread that performs matrix multiplication using the hardware accelerator and interrupts
 * 
 * @param mainId 
 * @param unused1 
 * @param unused2 
 */
void thread_accelerator(void *mainIdPtr, void *unused1, void *unused2)
{
    k_tid_t mainId = (k_tid_t) mainIdPtr;
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    int addressOffset = MATRIX1_SIZE + MATRIX2_SIZE + RESULT_POOL_SIZE + RESULT_HW_SIZE;

    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        int resultAddr = mat2Addr + MATRIX2_SIZE + RESULT_POOL_SIZE;

        multiply_mat_hw(mat1Addr, mat2Addr, resultAddr, MAT1ROWS, MAT1COLS, MAT2COLS);
        k_sem_take(&accel_sem, K_FOREVER);
    }

    k_thread_resume(mainId);
}