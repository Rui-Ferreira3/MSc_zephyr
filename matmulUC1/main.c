#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 10       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 9
#define SLEEPTIME   1000

int *acceleratorBusyFlag = (int *)ACCELERATOR_BUSY_FLAG_ADDR;

// int total_sw_time, total_hw_time;

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
    printf("Installing ISR...\n");
    my_isr_installer();

    head = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(head == NULL) printf("Error allocating memory for matmul linked list head!\n");
    init_matmul(head, 0, 0, 0);

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
    
    free_matmul(head);

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
    k_sem_give(&accel_thread_sem);
}

void thread_software() {
    while(1) {
        printf("Doing stuff...\n");

        printf("Defining new matmul...\n");
        struct matmul *newNode = (struct matmul *)k_malloc(sizeof(struct matmul));
        init_matmul(newNode, 8, 8, 8);
        fill_matmul(newNode);

        push_matmul(head, newNode);
        printf("New matmul added to the stack!\n");
        k_sem_give(&accel_thread_sem);
    }
}

void thread_accelerator() {
    while(1) {
        k_sem_take(&accel_thread_sem, K_FOREVER);
        printf("Inside thread!\n");

        if(calculatingNode != NULL) {
            save_matmul(calculatingNode);
            free_matmul(calculatingNode);
            calculatingNode = NULL;
        }

        if(head->next != NULL) {
            perform_matmul(head, &calculatingNode);
        }
    }
}