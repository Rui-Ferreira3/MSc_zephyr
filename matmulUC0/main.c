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


    /* perform NUM_MATMULS in hardware*/
    printf("\nPerforming hardware matrix multiplication...\n");
    node = head->next;
    start_hw_ms = k_uptime_get();
    for(int i=0; i<NUM_MATMULS; i++) {
        multiply_mat_hw((int)node->mat1, (int)node->mat2, (int)node->resultHW, node->mat1Rows, node->mat1Cols, node->mat2Cols);
        // print_mat(node->resultHW, node->mat1Rows, node->mat2Cols);
        node = node->next;
    }
    finish_hw_ms = k_uptime_get();
    time_hw = finish_hw_ms - start_hw_ms;
    printf("Completed hardware matrix multiplication!\n");


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

// void init_mats() {
//     for(int i=0; i<MATI_ROWS ; i++)
//         for(int j=0; j<MATI_COLS; j++) {
//             if(i == j) matI[i*MATI_COLS + j] = 1;
//             else matI[i*MATI_COLS + j] = 0;
//             mat1[i*MATI_COLS + j] = 1;
//             mat2[i*MATI_COLS + j] = 2;
//             mat3[i*MATI_COLS + j] = i;
//         }
// }

// void define_mats() {
//     mat_address_2 = MATI_BASE_ADDRESS;
//     mat_rows_2 = MATI_ROWS;
//     mat_cols_2 = MATI_COLS;
//     matrix2 = matI;

//     switch (matrixID) {
//     case 1:
//         mat_address_1 = MAT1_BASE_ADDRESS;
//         mat_rows_1 = MAT1_ROWS;
//         mat_cols_1 = MAT1_COLS;
//         matrix1 = mat1;
//         break;
//     case 2:
//         mat_address_1 = MAT2_BASE_ADDRESS;
//         mat_rows_1 = MAT2_ROWS;
//         mat_cols_1 = MAT2_COLS;
//         matrix1 = mat2;
//         break;
//     case 3:
//         mat_address_1 = MAT3_BASE_ADDRESS;
//         mat_rows_1 = MAT3_ROWS;
//         mat_cols_1 = MAT3_COLS;
//         matrix1 = mat3;
//         break;
//     default:
//         mat_address_1 = MATI_BASE_ADDRESS;
//         mat_rows_1 = MATI_ROWS;
//         mat_cols_1 = MATI_COLS;
//         matrix1 = matI;
//     }
// }