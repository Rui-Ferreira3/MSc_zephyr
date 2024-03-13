#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 10       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 9
#define SLEEPTIME   1000

int *acceleratorBusyFlag = (int *)ACCELERATOR_BUSY_FLAG_ADDR;
int time_hw;

K_SEM_DEFINE(accel_thread_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(sw_thread_sem, 1, 1);	/* starts off "available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;
k_tid_t accel_thread_id;
K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;
k_tid_t sw_thread_id;

int main()
{
    int numErrors = 0, totalErrors=0;
    uint32_t start_sw_ms, finish_sw_ms, time_sw=0;
    struct matmul *node;

    printf("*** Starting matmul UC 1 ***\n\n");

    printf("Installing ISR...\n");
    my_isr_installer();

    /* initialize matmul queue head */
    head = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(head == NULL) printf("Error allocating memory for matmul linked list head!\n");
    init_matmul(head, 0, 0, 0);

    /* fill the accelerator inputs inside the matmul structure, and push the new node into the queue */
    printf("Defining %d matmuls and adding them to the queue...\n", NUM_MATMULS);
    for(int i=0; i<NUM_MATMULS; i++) {
        struct matmul *newNode = (struct matmul *)k_malloc(sizeof(struct matmul));
        if(newNode == NULL) {
            printf("Could not allocate space for matmul %d!\n", i);
            break;
        }
        int err = init_matmul(newNode, 8, 8, 8);
        if(err == -1) {
            printf("Could not allocate space for matmul %d!\n", i);
            break;
        }
        fill_matmul(newNode);
        push_matmul(head, newNode);
    }
    printf("%d matmuls added to the queue!\n", NUM_MATMULS);

    /* perform NUM_MATMULS in software */
    printf("\nPerforming software matrix multiplication...\n");
    node = head->next;
    start_sw_ms = k_uptime_get();
    for(int i=0; i<NUM_MATMULS; i++) {
        multiply_mat_sw(node->resultSW, node->mat1, node->mat2, node->mat1Rows, node->mat1Cols, node->mat2Cols);
        // print_mat(node->resultSW, node->mat1Rows, node->mat2Cols);
        node = node->next;
    }
    finish_sw_ms = k_uptime_get();
    time_sw = finish_sw_ms - start_sw_ms;
    printf("Completed software matrix multiplication!\n");

    /* starting software and accelerator threads*/
    accel_thread_id = k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, NULL, NULL, NULL,
        ACCEL_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");
    k_thread_start(&threadA_data);

    sw_thread_id = k_thread_create(&threadB_data, threadB_stack_area,
        K_THREAD_STACK_SIZEOF(threadB_stack_area),
        thread_software, NULL, NULL, NULL,
        SW_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadB_data, "thread_b");
    k_thread_start(&threadB_data);

    /* check if software and hardware matmul match */
    // node = head->next;
    // for(int i=0; i<NUM_MATMULS; i++) {
    //     numErrors = verify_matmul(node->resultHW, node->resultSW, node->mat1Rows, node->mat2Cols);
    //     totalErrors += numErrors;
    //     node = node->next;
    // }

    k_msleep(1000);

    time_hw =0;
    printf("\n%d matmuls done with %d errors!\n", NUM_MATMULS, numErrors);
    printf("Software took %u miliseconds\n", time_sw);
    printf("Hardware took %u miliseconds\n", time_hw);
    
    printf("\nFreeing memory...\n");
    free_queue(head);

    printf("\n*** Exiting matmul UC 1 ***\n");

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
    volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
    volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
    *acceleratorGIER = 0;
    *acceleratorIP_IER = 0;
    *acceleratorBusyFlag = 0;
    k_sem_give(&accel_thread_sem);
}


void thread_software() {
    printf("Inside software thread!\n");
    // while(1) {
    //     printf("Doing stuff...\n");

    //     printf("Defining new matmul...\n");
    //     struct matmul *newNode = (struct matmul *)k_malloc(sizeof(struct matmul));
    //     init_matmul(newNode, 8, 8, 8);
    //     fill_matmul(newNode);

    //     push_matmul(head, newNode);
    //     printf("New matmul added to the stack!\n");
    // }
}


void thread_accelerator() {
    printf("Inside accelerator thread!\n");
    // while(1) {
    //     k_sem_take(&accel_thread_sem, K_FOREVER);
    //     printf("Inside thread!\n");

    //     if(calculatingNode != NULL) {
    //         save_matmul(calculatingNode);
    //         free_matmul(calculatingNode);
    //         calculatingNode = NULL;
    //     }

    //     if(head->next != NULL) {
    //         perform_matmul(head, &calculatingNode);
    //     }
    // }
}