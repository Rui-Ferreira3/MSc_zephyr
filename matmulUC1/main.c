#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5

K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

int main()
{
    printf("*** Starting matmul UC 1 ***\n\n");

    printf("Installing ISR...\n");
    my_isr_installer();
    
    int addressOffset = MATRIX1_SIZE + MATRIX2_SIZE + RESULT_POOL_SIZE + RESULT_HW_SIZE;

    /* initialize the queue */
    printf("\nSaving %d matrix pairs to memory...\n", NUM_MULTIPLICATIONS);
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        create_mat(mat1Addr, MAT1ROWS, MAT1COLS);

        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        create_mat(mat2Addr, MAT1ROWS, MAT1COLS);
    }
    printf("%d saved to memory!\n", NUM_MULTIPLICATIONS);

    /* perform NUM_MULTIPLICATIONS with pooling */
    printf("\nPerforming matrix multiplication with pooling...\n");

    /* disable accelerator interrupts */
    *acceleratorGIER = 0x0;

    int start_p_ms = k_uptime_get();
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        int resultAddr = mat2Addr + MATRIX2_SIZE;

        multiply_mat_hw_pool(mat1Addr, mat2Addr, resultAddr, MAT1ROWS, MAT1COLS, MAT2COLS);
    }

    int finish_p_ms = k_uptime_get();
    int time_p = finish_p_ms - start_p_ms;

    /* enable accelerator interrupts */
    *acceleratorIP_ISR = 0x1;
    *acceleratorGIER = 0x1;

    printf("Completed matrix multiplication with pooling!\n");
    printf("Execution time: %d ms\n", time_p);

    /* starting accelerator thread */
    k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, k_current_get(), NULL, NULL,
        ACCEL_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");
    k_thread_start(&threadA_data);

    printf("\nPerforming matrix multiplication using the hardware accelerator...\n");

    int start_hw_ms = k_uptime_get();

    k_thread_suspend(k_current_get());

    int finish_hw_ms = k_uptime_get();
    int time_hw = finish_hw_ms - start_hw_ms;

    printf("Completed matrix multiplication using the hardware accelerator!\n");
    printf("Execution time: %d ms\n", time_hw);


    /* check if software and hardware matmul match */
    printf("\nChecking if results match...\n");
    int numErrors = 0;
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        int resultPoolAddr = mat2Addr + MATRIX2_SIZE;
        int resultHWAddr = resultPoolAddr + RESULT_POOL_SIZE;

        numErrors += verify(resultPoolAddr, resultHWAddr, MAT1ROWS, MAT2COLS);
    }

    printf("\n%d operations done with %d errors!\n", NUM_MULTIPLICATIONS, numErrors);

    printf("\n*** Exiting matmul UC 1 ***\n");

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

void thread_accelerator(k_tid_t mainId, void *unused1, void *unused2)
{
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