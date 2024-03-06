#include "header.h"

#define MY_DEV_IRQ  15       /* device uses IRQ 15 */
#define MY_DEV_PRIO 10       /* device uses interrupt priority 5 */
#define THREAD_PRIO 7
#define SLEEPTIME   1000

int *acceleratorBusyFlag = (int *)ACCELERATOR_BUSY_FLAG_ADDR;

// int total_sw_time, total_hw_time;

K_MUTEX_DEFINE(matmul_mutex);
K_SEM_DEFINE(usingAccelerator_sem, 1, 1);	/* starts off "available" */
// K_SEM_DEFINE(thread_accelerator_sem, 0, 1);	/* starts off "not available" */
// K_SEM_DEFINE(thread_software_sem, 1, 1);	/* starts off "available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;
K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

int main()
{
    // struct matmul *newNode, *currentNode, *node;
    printf("Installing ISR...\n");
    my_isr_installer();

    head = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(head == NULL) printf("Error allocating memory for matmul linked list head!\n");
    init_matmul(head, 0, 0, 0);

    while(1) {
        printf("Doing stuff...\n");
		k_msleep(SLEEPTIME);

        printf("Defining new matmul...\n");
        struct matmul *newNode = (struct matmul *)k_malloc(sizeof(struct matmul));
        init_matmul(newNode, 8, 8, 8);
        fill_matmul(newNode);

        // k_mutex_lock(&matmul_mutex, K_FOREVER);
        push_matmul(head, newNode);
        // k_mutex_unlock(&matmul_mutex);
        printf("New matmul added to the stack!\n");

        k_thread_create(&threadB_data, threadB_stack_area,
            K_THREAD_STACK_SIZEOF(threadB_stack_area),
            thread_accelerator, NULL, NULL, NULL,
            THREAD_PRIO, 0, K_FOREVER);
        k_thread_name_set(&threadB_data, "thread_b");
        k_thread_start(&threadB_data);
    }
    
    free_matmul(head);

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

    k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, NULL, NULL, NULL,
        MY_DEV_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");
    k_thread_start(&threadA_data);
}

void thread_accelerator() {
    printf("Inside thread!\n");

    if(calculatingNode != NULL) {
        save_matmul(calculatingNode);
        free_matmul(calculatingNode);
        calculatingNode = NULL;
    }

    if(head->next != NULL && calculatingNode == NULL) {
        perform_matmul(head, &calculatingNode);
    }
}