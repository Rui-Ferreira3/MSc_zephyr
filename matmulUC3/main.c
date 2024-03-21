#include "header.h"

#define ACCEL_IRQ  15       /* device uses IRQ 15 */
#define ACCEL_PRIO 7       /* device uses interrupt priority 7 */

#define ACCEL_THREAD_PRIO 5
#define SOFTWARE_THREAD_PRIO 8
#define RESET_THREAD_PRIO 10

int finish_hw_ms, start_hw_ms, time_hw=0, time_p=0;
int completed[NUM_THREADS];

static K_THREAD_STACK_ARRAY_DEFINE(stacks, NUM_THREADS+2, STACKSIZE);
static struct k_thread threads[NUM_THREADS+2];
k_tid_t thread_ids[NUM_THREADS];

// struct k_pipe pipe_array[NUM_THREADS];
// K_PIPE_DEFINE(accel_pipe, 0, 4);
K_MSGQ_DEFINE(accel_msgq, sizeof(struct message), 10, 4);

K_SEM_DEFINE(reset_thread_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(accel_thread_sem, 0, 1);	/* starts off "not available" */
K_SEM_DEFINE(sw_thread_sem, 1, 1);	/* starts off "available" */
K_SEM_DEFINE(accel_sem, 0, 1);	/* starts off "not available" */

K_MUTEX_DEFINE(head_mutex);
K_MUTEX_DEFINE(completedHead_mutex);



int main()
{
    printf("*** Starting matmul UC 3 ***\n\n");

    printf("Installing ISR...\n");
    isr_installer();

    /* initialize matmul overal queue head */
    head = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(head == NULL) printf("Error allocating memory for matmul linked list head!\n");
    init_matmul(head, 0, 0, 0);

    /* fill the queue with a set of matmulls */
    printf("\nDefining %d matmuls and adding them to the queue...\n", NUM_MATMULS);
    fill_queue(head, NUM_MATMULS);
    printf("%d matmuls added to the queue!\n", NUM_MATMULS);


    /* initialize completed matmul queue head */
    completedHead = (struct matmul *)k_malloc(sizeof(struct matmul));
    if(completedHead == NULL) printf("Error allocating memory for completed matmul linked list head!\n");
    init_matmul(completedHead, 0, 0, 0);


    /* perform NUM_MATMULS in software */
    printf("\nPerforming matrix multiplication with pooling...\n");
    time_p = pooling_matmul(head);
    printf("Completed matrix multiplication with pooling!\n");

    for(int i=0; i<NUM_THREADS; i++)
        completed[i] = 0;

    printf("\nPerforming hardware matrix multiplication...\n");
    start_hw_ms = k_uptime_get();

    /* start all threads */
    for(int i=0; i<NUM_THREADS+2; i++) {
        char tname[CONFIG_THREAD_MAX_NAME_LEN];

        if(i == NUM_THREADS) {
            k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                    thread_reset, INT_TO_POINTER(i), NULL, NULL,
                    RESET_THREAD_PRIO, K_USER, K_FOREVER);
            snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "reset");
        }else if(i == NUM_THREADS+1) {
            k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                    thread_accelerator, INT_TO_POINTER(i), NULL, NULL,
                    ACCEL_THREAD_PRIO, K_USER, K_FOREVER);
            snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "accelerator");
        }else {
            // k_pipe_init(&pipe_array[i], NULL, 0);

            thread_ids[i] = k_thread_create(&threads[i], &stacks[i][0], STACKSIZE,
                    thread_software, INT_TO_POINTER(i), NULL, NULL,
                    SOFTWARE_THREAD_PRIO, K_USER, K_FOREVER);
            snprintk(tname, CONFIG_THREAD_MAX_NAME_LEN, "thread %d", i);
        }
		k_thread_name_set(&threads[i], tname);
        k_thread_start(&threads[i]);
    }

    return 0;
}

/**
 * @brief installs accelerator isr
 * 
 */
void isr_installer(void) {
    *acceleratorGIER = 0x1;
    *acceleratorIP_IER = 0x1;

    ARCH_IRQ_CONNECT(ACCEL_IRQ, ACCEL_PRIO, accel_isr, NULL, 0);
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
void accel_isr(const void *arg) {
    *acceleratorIP_ISR = 0x1;
    
    k_sem_give(&accel_sem);
    // k_sem_give(&accel_thread_sem);
}

void thread_reset(void *id, void *unused1, void *unused2) {
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    int resetHead;

    printf("Inside thread reset!\n");

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

            k_thread_priority_set(k_current_get(), ACCEL_THREAD_PRIO-1);
            for(int i=0; i<NUM_THREADS; i++) {
                k_thread_resume(thread_ids[i]);
                completed[i] = 0;
            }
            printf("\nPerforming hardware matrix multiplication...\n");
            start_hw_ms = k_uptime_get();
            k_thread_priority_set(k_current_get(), RESET_THREAD_PRIO);
        }
    }
}

void thread_software(void *id, void *unused1, void *unused2)
{
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    int resetHead=0;
    int my_id = POINTER_TO_INT(id);

    unsigned char buffer[70000];
    struct message *msg = (struct message *)buffer;
    msg->sender_id = my_id;
    size_t bytes_written, bytes_read;

    
    printf("Inside thread software %d!\n", my_id);

    while(1) {
        // k_mutex_lock(&head_mutex, K_FOREVER);
        // if(head->next == NULL) {
        //     resetHead = 1;
        // }else {
        //     resetHead = 0;
        //     pop2_matmul(head, &msg->node);
        // }
        // k_mutex_unlock(&head_mutex);

        // if(resetHead) {
        //     completed[my_id] = 1;
        //     k_thread_suspend(thread_ids[my_id]);
        // }else {
        //     k_pipe_put(&pipe_central, (void *)msg, sizeof(msg), &bytes_written, sizeof(struct message), K_NO_WAIT);
        //     printf("Thread %d sent message to central thread!\n", my_id);

        //     k_pipe_get(&pipe_array[my_id], buffer, sizeof(buffer), &bytes_read, sizeof(struct message), K_FOREVER);
        //     printf("Thread %d received a reply!\n", my_id);

        //     k_mutex_lock(&completedHead_mutex, K_FOREVER);
        //     push2_matmul(completedHead, msg->node);
        //     k_mutex_unlock(&completedHead_mutex);
        // }
    }
    
}

void thread_accelerator(void *id, void *unused1, void *unused2) {
    ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);

    unsigned char buffer[70000];
    struct message *msg = (struct message *)buffer;
    size_t bytes_written, bytes_read;

    printf("Inside thread accelerator!\n");

    while(1) {
        // k_pipe_get(&accel_pipe, buffer, sizeof(buffer), &bytes_read, sizeof(struct message), K_FOREVER);
        // printf("Received message from %d\n", msg->sender_id);

        // hardware_matmul(msg->node);
        // k_sem_take(&accel_sem, K_FOREVER);

        // k_pipe_put(&pipe_array[msg->sender_id], (void *)msg, sizeof(msg), &bytes_written, sizeof(struct message), K_NO_WAIT);
        // printf("Sending reply to %d...\n", msg->sender_id);
    }
}