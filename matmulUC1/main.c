#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 10

int finish_hw_ms, start_hw_ms, time_hw=0, time_p=0;
int hwCount=0;

K_SEM_DEFINE(accel_thread_sem, 0, 1);	/* starts off "available" */
K_SEM_DEFINE(reset_thread_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_MUTEX_DEFINE(head_mutex);
K_MUTEX_DEFINE(completedHead_mutex);

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

int main()
{
    printf("*** Starting matmul UC 1 ***\n\n");

    printf("Installing ISR...\n");
    my_isr_installer();


    /* initialize matmul queue head */
    head = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(head == NULL) printf("Error allocating memory for matmul linked list head!\n");
    init_matmul(head, 0, 0, 0);


    /* initialize completed matmul queue head */
    completedHead = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(completedHead == NULL) printf("Error allocating memory for completed matmul linked list head!\n");
    init_matmul(completedHead, 0, 0, 0);


    /* fill the queue with a set of matmulls */
    printf("\nDefining %d matmuls and adding them to the queue...\n", NUM_MATMULS);
    fill_queue(head, NUM_MATMULS);
    printf("%d matmuls added to the queue!\n", NUM_MATMULS);

    /* perform NUM_MATMULS in software */
    printf("\nPerforming matrix multiplication with pooling...\n");
    time_p = pooling_matmul(head);
    printf("Completed matrix multiplication with pooling!\n");


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

    // printf("\nPerforming hardware matrix multiplication...\n");
    // struct matmul *node;
    // while(head->next != NULL) {
    //     pop2_matmul(head, &node);
    //     node->next=NULL;
    //     printf("\nmat1: %f mat2: %f result: %f\n", node->mat1[0], node->mat2[0], node->resultSW[0]);
    //     hardware_matmul(node);
    //     k_sem_take(&accel_sem, K_FOREVER);
    //     printf("result %p: %f\n", node->resultHW, node->resultHW[0]);
    //     push2_matmul(completedHead, node);
    //     node=NULL;
    // }

    // int numErrors = verify_queue(completedHead, NUM_MATMULS);
    // printf("Completed hardware matrix multiplication with %d errors!\n", numErrors);

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

void thread_reset() {
    int count=0;
    int resetHead=0;
    struct matmul *node;

    while(1) {
        k_mutex_lock(&head_mutex, K_FOREVER);
        if(head->next == NULL) {
            resetHead = 1;
        }else {
            resetHead = 0;
            pop2_matmul(head, &node);
        }
        k_mutex_unlock(&head_mutex);

        if(resetHead) {
            k_sem_take(&reset_thread_sem, K_FOREVER);
            finish_hw_ms = k_uptime_get();
            time_hw = finish_hw_ms - start_hw_ms;
            printf("Completed hardware matrix multiplication!\n");

            /* check if pooling and usecase matmul match */
            printf("\nVerifying results...\n");
            int numErrors = verify_queue(completedHead, NUM_MATMULS);
            printf("\nReseting the queue...\n");
            reset_queue(head, completedHead);

            printf("\n%d matmuls done with %d errors!\n", NUM_MATMULS, numErrors);
            printf("Pooling took %u miliseconds\n", time_p);
            printf("Hardware took %u miliseconds\n", time_hw);

            printf("Performed %d in sw.\n", count);
            printf("Performed %d in hw.\n", hwCount);
            count = 0;
            hwCount = 0;

            k_sem_give(&accel_thread_sem);
        }else {
            count++;
            node->next=NULL;

            multiply_mat_sw(node->resultHW, node->mat1, node->mat2, node->mat1Rows, node->mat1Cols, node->mat2Cols);
            // printf("\nmat1: %f mat2: %f result: %f\n", node->mat1[0], node->mat2[0], node->resultSW[0]);
            // printf("result %p: %f\n", node->resultHW, node->resultHW[0]);
            
            k_mutex_lock(&completedHead_mutex, K_FOREVER);
            push2_matmul(completedHead, node);
            k_mutex_unlock(&completedHead_mutex);
        }
    }
}

void thread_accelerator() {
    int resetHead=0;
    struct matmul *node;

    printf("\nPerforming hardware matrix multiplication...\n");
    start_hw_ms = k_uptime_get();
    while(1) {
        k_mutex_lock(&head_mutex, K_FOREVER);
        if(head->next == NULL) {
            resetHead = 1;
        }else {
            resetHead = 0;
            pop2_matmul(head, &node);
        }
        k_mutex_unlock(&head_mutex);

        if(resetHead) {
            k_sem_give(&reset_thread_sem);
            k_sem_take(&accel_thread_sem, K_FOREVER);
            printf("\nPerforming hardware matrix multiplication...\n");
            start_hw_ms = k_uptime_get();
        }else {
            hwCount++;
            node->next=NULL;

            hardware_matmul(node);
            k_sem_take(&accel_sem, K_FOREVER);
            
            k_mutex_lock(&completedHead_mutex, K_FOREVER);
            push2_matmul(completedHead, node);
            k_mutex_unlock(&completedHead_mutex);
        }
    }
}