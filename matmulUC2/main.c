#include "header.h"

#define MY_DEV_IRQ  15       /* device uses IRQ 15 */
#define MY_DEV_PRIO 5       /* device uses interrupt priority 5 */
#define THREAD_PRIO 7
#define SLEEPTIME   500

int *acceleratorBusyFlag = (int *)ACCELERATOR_BUSY_FLAG_ADDR;

float *matI = (float *)MATI_BASE_ADDRESS;
float *mat1 = (float *)MAT1_BASE_ADDRESS;
float *mat2 = (float *)MAT2_BASE_ADDRESS;
float *mat3 = (float *)MAT3_BASE_ADDRESS;
float *resultSw = (float *)RESULT_SW_BASE_ADDRESS;
float *resultHw = (float *)RESULT_HW_BASE_ADDRESS;

int matrixID;

int mat_address_1;
int mat_rows_1;
int mat_cols_1;
float *matrix1;

int mat_address_2;
int mat_rows_2;
int mat_cols_2;
float *matrix2;

int total_sw_time, total_hw_time;

K_SEM_DEFINE(usingAccelerator_sem, 1, 1);	/* starts off "available" */
K_SEM_DEFINE(thread_accelerator_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(thread_software_sem, 1, 1);	/* starts off "available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

int main()
{
    printf("Installing ISR...\n");
    my_isr_installer();

    printf("Defining matrices for matmul...\n");
    init_mats();

    k_thread_create(&threadA_data, threadA_stack_area,
            K_THREAD_STACK_SIZEOF(threadA_stack_area),
            thread_accelerator, NULL, NULL, NULL,
            THREAD_PRIO, 0, K_FOREVER);
	k_thread_name_set(&threadA_data, "thread_accelerator");

    k_thread_create(&threadB_data, threadB_stack_area,
			K_THREAD_STACK_SIZEOF(threadB_stack_area),
			thread_software, NULL, NULL, NULL,
			THREAD_PRIO, 0, K_FOREVER);
	k_thread_name_set(&threadB_data, "thread_b");

    k_thread_start(&threadA_data);
	k_thread_start(&threadB_data);

    return 0;
}

void my_isr_installer(void) {
    ARCH_IRQ_CONNECT(MY_DEV_IRQ, MY_DEV_PRIO, my_isr, NULL, 0);
    arch_irq_enable(MY_DEV_IRQ);
}

void my_isr(const void *arg) {
    volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
    volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
    *acceleratorGIER = 0;
    *acceleratorIP_IER = 0;
    *acceleratorBusyFlag = 0;
    k_sem_give(&usingAccelerator_sem);
    return;
}

void init_mats() {
    for(int i=0; i<MATI_ROWS ; i++)
        for(int j=0; j<MATI_COLS; j++) {
            if(i == j) matI[i*MATI_COLS + j] = 1;
            else matI[i*MATI_COLS + j] = 0;
            mat1[i*MATI_COLS + j] = 1;
            mat2[i*MATI_COLS + j] = 2;
            mat3[i*MATI_COLS + j] = i;
        }
}

void define_mats() {
    mat_address_1 = MATI_BASE_ADDRESS;
    mat_rows_1 = MATI_ROWS;
    mat_cols_1 = MATI_COLS;
    matrix1 = matI;

    switch (matrixID) {
    case 1:
        mat_address_2 = MAT1_BASE_ADDRESS;
        mat_rows_2 = MAT1_ROWS;
        mat_cols_2 = MAT1_COLS;
        matrix2 = mat1;
        break;
    case 2:
        mat_address_2 = MAT2_BASE_ADDRESS;
        mat_rows_2 = MAT2_ROWS;
        mat_cols_2 = MAT2_COLS;
        matrix2 = mat2;
        break;
    case 3:
        mat_address_2 = MAT3_BASE_ADDRESS;
        mat_rows_2 = MAT3_ROWS;
        mat_cols_2 = MAT3_COLS;
        matrix2 = mat3;
        break;
    default:
        mat_address_2 = MATI_BASE_ADDRESS;
        mat_rows_2 = MATI_ROWS;
        mat_cols_2 = MATI_COLS;
        matrix2 = matI;
    }
}