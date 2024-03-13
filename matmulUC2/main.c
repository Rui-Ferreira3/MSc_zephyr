#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 10       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 9
#define SLEEPTIME   1000

int *acceleratorBusyFlag = (int *)ACCELERATOR_BUSY_FLAG_ADDR;

// int total_sw_time, total_hw_time;

K_MUTEX_DEFINE(matmul_mutex);
K_SEM_DEFINE(usingAccelerator_sem, 1, 1);	/* starts off "available" */
// K_SEM_DEFINE(accel_thread_sem, 0, 1);	/* starts off "not available" */
// K_SEM_DEFINE(sw_thread_sem, 1, 1);	/* starts off "available" */

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

int main()
{
    // struct matmul *newNode, *currentNode, *node;
    printf("Installing ISR...\n");
    my_isr_installer();

    k_thread_create(&threadA_data, threadA_stack_area,
        K_THREAD_STACK_SIZEOF(threadA_stack_area),
        thread_accelerator, NULL, NULL, NULL,
        ACCEL_THREAD_PRIO, 0, K_FOREVER);
    k_thread_name_set(&threadA_data, "thread_a");
    k_thread_start(&threadA_data);

    return 0;
}

void my_isr_installer(void) {
    ARCH_IRQ_CONNECT(ACCEL_IRQ, ACCEL_PRIO, my_isr, NULL, 0);
    arch_irq_enable(ACCEL_IRQ);
}

void my_isr(const void *arg) {
    volatile int *acceleratorGIER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x04);
    volatile int *acceleratorIP_IER = (int *)(ACCELERATOR_BASE_ADDRESS + 0x08);
    *acceleratorGIER = 0;
    *acceleratorIP_IER = 0;
    *acceleratorBusyFlag = 0;
    k_sem_give(&usingAccelerator_sem);
}

void thread_accelerator() {
    // int my_id = POINTER_TO_INT(id);

    struct matmul *head, *calculatingNode;

    head = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(head == NULL) printf("Error allocating memory for matmul linked list head!\n");
    init_matmul(head, 0, 0, 0);

    while(1) {
        printf("Doing stuff...\n");

        printf("Defining new matmul...\n");
        struct matmul *newNode = (struct matmul *)k_malloc(sizeof(struct matmul));
        init_matmul(newNode, 8, 8, 8);
        fill_matmul(newNode);

        push_matmul(head, newNode);
        printf("New matmul added to the stack!\n");

        if(head->next != NULL && !acceleratorBusyFlag) {
            perform_matmul(head, &calculatingNode);

            // wait for interrupt

            save_matmul(calculatingNode);
            free_matmul(calculatingNode);
            calculatingNode = NULL;
        }
    }
    
    free_matmul(head);
}