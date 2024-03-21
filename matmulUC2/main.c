#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 5 */
#define ACCEL_THREAD_PRIO 5
#define SW_THREAD_PRIO 10

int finish_hw_ms, start_hw_ms, time_hw=0, time_p=0;
int *completed;
k_tid_t thread_ids[NUM_THREADS];

K_SEM_DEFINE(accel_thread_sem, 1, 1);	/* starts off "available" */
K_SEM_DEFINE(reset_thread_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_MUTEX_DEFINE(head_mutex);
K_MUTEX_DEFINE(completedHead_mutex);

static K_THREAD_STACK_ARRAY_DEFINE(stacks, NUM_THREADS+1, STACKSIZE);
static struct k_thread threads[NUM_THREADS+1];

// K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
// static struct k_thread threadB_data;

int main()
{
    printf("*** Starting matmul UC 2 ***\n\n");

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

    completed = (int *)k_malloc(sizeof(int));
    for(int i=0; i<NUM_THREADS; i++)
        completed[i] = 0;
    
    printf("\nPerforming hardware matrix multiplication...\n");
    start_hw_ms = k_uptime_get();

    /* start all accelerator threads */
    for(int i=0; i<NUM_THREADS+1; i++) {
        char tname[CONFIG_THREAD_MAX_NAME_LEN];

        if(i == NUM_THREADS) {
            k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                    thread_reset, INT_TO_POINTER(i), NULL, NULL,
                    SW_THREAD_PRIO, K_USER, K_FOREVER);
            snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "reset");
        }else {
            thread_ids[i] = k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                    thread_accelerator, INT_TO_POINTER(i), NULL, NULL,
                    ACCEL_THREAD_PRIO, K_USER, K_FOREVER);
            snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "Philosopher %d", i);
        }
		k_thread_name_set(&threads[i], tname);
        k_thread_start(&threads[i]);
    }

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
    k_sem_give(&accel_thread_sem);
}

void thread_reset(void *id, void *unused1, void *unused2) {
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    int resetHead;

    while(1) {
        resetHead = 0;
        for(int i=0; i<NUM_THREADS; i++)
            if(completed[i] == 1) resetHead++;

        if(resetHead == NUM_THREADS) {
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

            k_thread_priority_set(k_current_get(), ACCEL_PRIO-1);
            for(int i=0; i<NUM_THREADS; i++) {
                k_thread_resume(thread_ids[i]);
                completed[i] = 0;
            }
            printf("\nPerforming hardware matrix multiplication...\n");
            start_hw_ms = k_uptime_get();
            k_thread_priority_set(k_current_get(), SW_THREAD_PRIO);
        }
    }
}

void thread_accelerator(void *id, void *unused1, void *unused2) {
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    int resetHead=0;
    struct matmul *node;

    int my_id = POINTER_TO_INT(id);

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
            // printf("Suspending %d\n", my_id);
            completed[my_id] = 1;
            k_thread_suspend(thread_ids[my_id]);
        }else {
            k_sem_take(&accel_thread_sem, K_FOREVER);
            hardware_matmul(node);
            k_sem_take(&accel_sem, K_FOREVER);
            
            k_mutex_lock(&completedHead_mutex, K_FOREVER);
            push2_matmul(completedHead, node);
            k_mutex_unlock(&completedHead_mutex);
        }
    }
}