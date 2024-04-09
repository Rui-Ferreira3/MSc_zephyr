#include "header.h"

int main()
{
    int numErrors = 0, totalErrors;
    uint32_t start_sw_ms, finish_sw_ms, time_sw=0;
    uint32_t start_hw_ms, finish_hw_ms, time_hw=0;
    struct matmul *head, *node;

    printf("*** Starting matmul UC 0 ***\n\n");

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
        int err = init_matmul(newNode, MAT1ROWS, MAT1COLS, MAT2COLS);
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
        // printf("SW %d\n", i);
        multiply_mat_sw(node->resultSW, node->mat1, node->mat2, node->mat1Rows, node->mat1Cols, node->mat2Cols);
        // print_mat(node->resultSW, node->mat1Rows, node->mat2Cols);
        node = node->next;
    }
    finish_sw_ms = k_uptime_get();
    time_sw = finish_sw_ms - start_sw_ms;
    printf("Completed software matrix multiplication!\n");


    /* perform NUM_MATMULS in hardware*/
    printf("\nPerforming pooling matrix multiplication...\n");
    node = head->next;
    start_hw_ms = k_uptime_get();
    for(int i=0; i<NUM_MATMULS; i++) {
        // printf("HW %d\n", i);
        multiply_mat_hw((int)node->mat1, (int)node->mat2, (int)node->resultHW, node->mat1Rows, node->mat1Cols, node->mat2Cols);
        // print_mat(node->resultHW, node->mat1Rows, node->mat2Cols);
        node = node->next;
    }
    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;
    printf("Completed pooling matrix multiplication!\n");


    /* check if software and hardware matmul match */
    node = head->next;
    for(int i=0; i<NUM_MATMULS; i++) {
        numErrors = verify_matmul(node->resultHW, node->resultSW, node->mat1Rows, node->mat2Cols);
        totalErrors += numErrors;
        node = node->next;
    }


    printf("\n%d matmuls done with %d errors!\n", NUM_MATMULS, numErrors);
    printf("Software took %u miliseconds\n", time_sw);
    printf("Hardware took %u miliseconds\n", time_hw);

    printf("\nFreeing memory...\n");
    free_queue(head);

    printf("\n*** Exiting matmul UC 0 ***\n");

    return 0;
}