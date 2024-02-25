#include "header.h"

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* delay between greetings (in ms) */
#define SLEEPTIME 500

/* define semaphores */
K_SEM_DEFINE(usingAccelerator_sem, 1, 1);	/* starts off "available" */
K_SEM_DEFINE(usingData_sem, 0, 1);	/* starts off "not available" */

int mat_address_1;
int mat_rows_1;
int mat_cols_1;
float *matrix1;

int mat_address_2;
int mat_rows_2;
int mat_cols_2;
float *matrix2;

int result_hw_address;
int result_sw_address;

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

int main()
{
    init_mats();

    mat_address_1 = MATI_BASE_ADDRESS;
    mat_rows_1 = MATI_ROWS;
    mat_cols_1 = MATI_COLS;
    matrix1 = matI;

    mat_address_2 = MAT1_BASE_ADDRESS;
    mat_rows_2 = MAT1_ROWS;
    mat_cols_2 = MAT1_COLS;
    matrix2 = mat1;

    software(NUM_MATMULS);

    k_thread_create(&threadA_data, threadA_stack_area,
            K_THREAD_STACK_SIZEOF(threadA_stack_area),
            threadAccelerator, NULL, NULL, NULL,
            PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadA_data, "thread_accelerator");

    k_thread_create(&threadB_data, threadB_stack_area,
			K_THREAD_STACK_SIZEOF(threadB_stack_area),
			threadData, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadB_data, "thread_b");

    k_thread_start(&threadA_data);
	k_thread_start(&threadB_data);


    return 0;
}

void threadAccelerator() {
        int num_errors, total_errors=0;
        uint32_t start_ms, finish_ms;

        int result_hw_base_address = MEM_BASE_ADDRESS + mat_rows_1*mat_cols_2*sizeof(float);
        float *result_sw = (float *)(result_sw_address);
        float *result_hw = (float *)(result_hw_base_address);

    while (1) {
        k_sem_take(&usingAccelerator_sem, K_FOREVER);
        printf("\nStarting %d matmuls with hardware accelerator!\n", NUM_MATMULS);

        start_ms = k_uptime_get();
        for(int i=0; i<NUM_MATMULS; i++) {
            if (i%2 == 0)
                multiply_mat_hw(mat_address_1, mat_address_2, result_hw_base_address, mat_rows_1, mat_cols_1, mat_cols_2);
            else
                multiply_mat_hw(mat_address_2, mat_address_1, result_hw_base_address, mat_rows_2, mat_cols_2, mat_cols_1);

            num_errors = verify_matmul(result_hw, result_sw, mat_rows_1, mat_cols_2);
            total_errors += num_errors;
        }
        finish_ms = k_uptime_get();

        printf("Testing done with %d errors!\nTook %u miliseconds!\n", total_errors, finish_ms-start_ms);

        /* wait a while, then let other thread have a turn */
		k_busy_wait(100000);
		k_msleep(SLEEPTIME);
		k_sem_give(&usingData_sem);
    }
}

void threadData() {
    while(1) {
        k_sem_take(&usingData_sem, K_FOREVER);

        printf("Collecting data...\n");

        /* wait a while, then let other thread have a turn */
		k_busy_wait(100000);
		k_msleep(SLEEPTIME);
		k_sem_give(&usingAccelerator_sem);
    }
}

void software(int num_matmuls) {
    uint32_t start_ms, finish_ms;

    result_sw_address = MEM_BASE_ADDRESS;
    printf("Starting %d matmuls in software!\n", num_matmuls);
    start_ms = k_uptime_get();
    for(int i=0; i<num_matmuls; i++)
        multiply_mat_sw(matrix1, matrix2, result_sw_address, mat_rows_1, mat_cols_1, mat_cols_2);
    finish_ms = k_uptime_get();
    printf("Took %u miliseconds!\n", finish_ms-start_ms);
}

void init_mats() {
    for(int i=0; i<MATI_ROWS ; i++)
        for(int j=0; j<MATI_COLS; j++) {
            if(i == j) matI[i*MATI_COLS + j] = 1;
            mat1[i*MATI_COLS + j] = 1;
            mat2[i*MATI_COLS + j] = 2;
            mat3[i*MATI_COLS + j] = i;
        }
}