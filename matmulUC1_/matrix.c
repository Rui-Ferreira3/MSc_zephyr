#include "matrix.h"

/**
 * @brief initialize new matmul node
 * 
 * @param node 
 * @param rows1 
 * @param cols1 
 * @param cols2 
 * @return -1 if any error occured allocating memory; 0 otherwise
 */
int init_matmul(struct matmul *node, int rows1, int cols1, int cols2)
{
    node->mat1 = (float *)k_malloc(rows1*cols1*sizeof(float));
    if(node->mat1 == NULL) return -1;
    node->mat2 = (float *)k_malloc(cols1*cols2*sizeof(float));
    if(node->mat2 == NULL) return -1;
    node->resultHW = (float *)k_malloc(rows1*cols2*sizeof(float));
    if(node->resultHW == NULL) return -1;
    node->resultSW = (float *)k_malloc(rows1*cols2*sizeof(float));
    if(node->resultSW == NULL) return -1;

    node->mat1Rows = rows1;
    node->mat1Cols = cols1;
    node->mat2Cols = cols2;

    node->next = NULL;

    return 0;
}

/**
 * @brief fill the queue with num_nodes operations
 * 
 * @param head 
 * @param num_matmuls 
 */
void fill_queue(struct matmul *head, int num_matmuls)
{
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
        push2_matmul(head, newNode);
    }
}

/**
 * @brief fill both input matrixes in node
 * 
 * @param node 
 */
void fill_matmul(struct matmul *node)
{
    create_mat(node->mat1, node->mat1Rows, node->mat1Cols);
    create_mat(node->mat2, node->mat1Cols, node->mat2Cols);
}

/**
 * @brief Create a random matrix
 * 
 * @param matrix 
 * @param rows 
 * @param cols 
 */
void create_mat(float *matrix, int rows, int cols)
{
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            int random_integer = sys_rand32_get()%100;
            matrix[i*cols+j] = (float)random_integer / 10;
            // matrix[i*cols+j] = 1;
        }
    }
}

/**
 * @brief perform num_matmuls operations
 * 
 * @param head 
 * @param num_matmuls 
 * @return time in miliseconds
 */
// int software_matmul(struct matmul *head, int num_matmuls)
// {
//     uint32_t start_sw_ms, finish_sw_ms;
//     struct matmul *node;

//     node = head->next;
//     start_sw_ms = k_uptime_get();
//     for(int i=0; i<NUM_MATMULS; i++) {
//         printf("Matmul %d\n", i);
//         multiply_mat_sw(node->resultSW, node->mat1, node->mat2, node->mat1Rows, node->mat1Cols, node->mat2Cols);
//         // print_mat(node->resultSW, node->mat1Rows, node->mat2Cols);
//         node = node->next;
//     }
//     finish_sw_ms = k_uptime_get();
//     return finish_sw_ms - start_sw_ms;
// }

int software_matmul(struct matmul *head)
{
    uint32_t start_sw_ms, finish_sw_ms;
    struct matmul *node;

    node = head->next;
    int i = 0;
    start_sw_ms = k_uptime_get();
    while(node != NULL) {
        // printf("Matmul %d\n", i++);
        multiply_mat_hw_pool((int)node->mat1, 
            (int)node->mat2, 
            (int)node->resultSW,
            node->mat1Rows,
            node->mat1Cols,
            node->mat2Cols);
        // print_mat(node->resultSW, node->mat1Rows, node->mat2Cols);
        node = node->next;
    }
    finish_sw_ms = k_uptime_get();
    return finish_sw_ms - start_sw_ms;
}

/**
 * @brief multiply matrixes in node
 * 
 * @param calculatingNode 
 */
void hardware_matmul(struct matmul *calculatingNode)
{
    multiply_mat_hw((int)calculatingNode->mat1, 
        (int)calculatingNode->mat2, 
        (int)calculatingNode->resultHW,
        calculatingNode->mat1Rows,
        calculatingNode->mat1Cols,
        calculatingNode->mat2Cols);
}

/**
 * @brief verify all matmuls in queue
 * 
 * @param head 
 * @param num_matmuls 
 * @return int 
 */
int verify_queue(struct matmul *head, int num_matmuls)
{
    struct matmul *node = head->next;
    int numErrors=0, totalErrors=0;
    int i=0;
    while(node != NULL) {
        numErrors += verify_matmul(node);
        totalErrors += numErrors;
        if(numErrors != 0)
            printf("Errors in matmul %d\n", i++);
        node = node->next;
    }
    return numErrors;
}

/**
 * @brief compare mat1 and mat2
 * 
 * @param mat1 
 * @param mat2 
 * @param rows 
 * @param cols 
 * @return number of differences between mat1 and mat2
 */
int verify_matmul(struct matmul *node)
{
    int num_errors = 0;
    int rows = node->mat1Rows;
    int cols = node->mat2Cols;

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            float diff = node->resultHW[i*cols+j] - node->resultSW[i*cols+j];
            if(diff > 0.01) {
                num_errors++;
                // printf("error at %d %d: %f-%f %f\n", i, j, node->resultHW[i*cols+j], node->resultSW[i*cols+j], diff);
            }
        }
    }
    // print_mat(mat1, rows, cols);
    // print_mat(mat2, rows, cols);

    return num_errors;
}

/**
 * @brief print matrix
 * 
 * @param matrix 
 * @param rows 
 * @param cols 
 */
void print_mat(float *matrix, int rows, int cols)
{
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            printf("%f\n", matrix[i*cols+j]);
        }
        printk("\n");
    }
    printk("\n\n");
}

/**
 * @brief push new node at the end of the queue
 * 
 * @param head 
 * @param newNode 
 */
void push_matmul(struct matmul *head, struct matmul *newNode)
{
    if (head == NULL) {
        printf("Can\'t push, because head is empty\n");
    } else {
        struct matmul *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

/**
 * @brief push new node at beggining of queue
 * 
 * @param head 
 * @param newNode 
 */
void push2_matmul(struct matmul *head, struct matmul *newNode)
{
     if (head == NULL) {
        printf("Can\'t push, because head is empty\n");
    } else {
        newNode->next = head->next;
        head->next = newNode;
    }
}

/**
 * @brief remove node from the end of the queue
 * 
 * @param head 
 * @param node 
 */
void pop_matmul(struct matmul *head, struct matmul **node)
{
    if(head->next == NULL) {
        printf("Stack is empty!\n");
        return;
    }else {
        struct matmul *current = head;
        while (current->next->next != NULL) {
            current = current->next;
        }
        *node = current->next;
        current->next = NULL;
    }
}

/**
 * @brief remove first node from the queue
 * 
 * @param head 
 * @param node 
 */
void pop2_matmul(struct matmul *head, struct matmul **node)
{
    if (head->next == NULL) {
        printf("Stack is empty!\n");
        return;
    }
    
    *node = head->next;

    if(head->next->next != NULL)
        head->next = head->next->next;
    else
        head->next = NULL;
}

int reset_queue(struct matmul *head, struct matmul *completedHead)
{
    struct matmul *node;

    while(completedHead->next != NULL) {
        pop2_matmul(completedHead, &node);

        k_free(node->resultHW);
        node->resultHW = (float *)k_malloc(node->mat1Rows*node->mat2Cols*sizeof(float));
        if(node->resultHW == NULL) return -1;

        k_free(node->resultSW);
        node->resultSW = (float *)k_malloc(node->mat1Rows*node->mat2Cols*sizeof(float));
        if(node->resultSW == NULL) return -1;

        node->next = NULL;

        push2_matmul(head, node);
    }

    return 0;
}

/**
 * @brief free memory used buy a single node
 * 
 * @param node 
 */
void free_matmul(struct matmul *node)
{
    k_free(node->mat1);
    k_free(node->mat2);
    k_free(node->resultHW);
    k_free(node->resultSW);
    node->next = NULL;
    k_free(node);
}

/**
 * @brief free full queue
 * 
 * @param head 
 */
void free_queue(struct matmul *head)
{
    struct matmul *node;
    while(head->next != NULL) {
        pop2_matmul(head, &node);
        free_matmul(node);
    }
}