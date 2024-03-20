#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 10
#define SLEEPTIME   1000

int *acceleratorBusyFlag = (int *)ACCELERATOR_BUSY_FLAG_ADDR;
int finish_hw_ms, start_hw_ms, time_hw=0, time_sw=0;
int hwCount=0;
int resetHead=0;

K_SEM_DEFINE(accel_thread_sem, 1, 1);	/* starts off "available" */
K_SEM_DEFINE(reset_thread_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

int main()
{
    printf("*** Starting matmul UC 1 ***\n\n");

    printf("Installing ISR...\n");
    my_isr_installer();

    *acceleratorBusyFlag = 0;

    /* initialize matmul queue head */
    head = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(head == NULL) printf("Error allocating memory for matmul linked list head!\n");
    init_matmul(head, 0, 0, 0);

    /* initialize completed matmul queue head */
    completedHead = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(completedHead == NULL) printf("Error allocating memory for completed matmul linked list head!\n");
    init_matmul(completedHead, 0, 0, 0);

    // /* fill the queue with a set of matmulls */
    // printf("\nDefining %d matmuls and adding them to the queue...\n", NUM_MATMULS);
    // fill_queue(head, NUM_MATMULS);
    // printf("%d matmuls added to the queue!\n", NUM_MATMULS);


    // /* perform NUM_MATMULS in software */
    // printf("\nPerforming software matrix multiplication...\n");
    // time_sw = software_matmul(head);
    // printf("Completed software matrix multiplication!\n");

    // printf("\nPerforming hardware matrix multiplication...\n");
    // start_hw_ms = k_uptime_get();

    /* starting software and accelerator threads*/
    k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, NULL, NULL, NULL,
        ACCEL_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");
    k_thread_start(&threadA_data);

    k_thread_create(&threadB_data, threadB_stack_area,
        K_THREAD_STACK_SIZEOF(threadB_stack_area),
        thread_reset, NULL, NULL, NULL,
        SW_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadB_data, "thread_b");
    k_thread_start(&threadB_data);

    return 0;
}

/**
 * @brief installs accelerator isr
 * 
 */
void my_isr_installer(void) {
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
    // volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
    volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
    // *acceleratorGIER = 0;
    *acceleratorIP_IER = 0;
    *acceleratorBusyFlag = 0;
    k_sem_give(&accel_sem);
}


void thread_reset() {
    int count=0;
    struct matmul *node;

    while(1) {
        printf("reset thread\n");
        k_sem_take(&reset_thread_sem, K_FOREVER);

        if(completedHead->next != NULL) {
            finish_hw_ms = k_uptime_get();
            time_hw = finish_hw_ms - start_hw_ms;
            printf("Completed hardware matrix multiplication!\n");

            /* check if software and hardware matmul match */
            int numErrors = verify_queue(completedHead, NUM_MATMULS);
            printf("\n%d matmuls done with %d errors!\n", NUM_MATMULS, numErrors);
            printf("Software took %u miliseconds\n", time_sw);
            printf("Hardware took %u miliseconds\n", time_hw);
            printf("Performed %d in sw.\n", count);
            printf("Performed %d in hw.\n", hwCount);
            count = 0;
            hwCount = 0;


            printf("\nFreeing memory...\n");
            free_queue(completedHead);
            k_sem_take(&reset_thread_sem, K_FOREVER);
        }

        /* fill the queue with a new set of matmulls */
        printf("\nDefining %d matmuls and adding them to the queue...\n", NUM_MATMULS);
        fill_queue(head, NUM_MATMULS);
        printf("%d matmuls added to the queue!\n", NUM_MATMULS);

        /* perform NUM_MATMULS in software */
        printf("\nPerforming software matrix multiplication...\n");
        time_sw = software_matmul(head);
        printf("Completed software matrix multiplication!\n");

        printf("\nPerforming hardware matrix multiplication...\n");
        start_hw_ms = k_uptime_get();

        k_sem_give(&accel_thread_sem);
    }
}


void thread_accelerator() {
    int i=0;
    struct matmul *node;

    while(1) {
        printf("ola\n");

        if(head->next != NULL) {
            printf("ola1\n");
            pop2_matmul(head, &node);
        }else {
            printf("ola2\n");
            k_sem_give(&reset_thread_sem);
            k_sem_take(&accel_thread_sem, K_FOREVER);
        }
        printf("ola\n");


        if(node != NULL) {
            printf("\nmatmul %d:\n", i++);
            hardware_matmul(node);
            k_sem_take(&accel_sem, K_FOREVER);
            // printf("hw: %f - sw: %f\n", node->resultHW[0], node->resultSW[0]);
            push2_matmul(completedHead, node);
            node = NULL;
        }
    }
}