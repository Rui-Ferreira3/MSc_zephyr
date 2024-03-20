#include "matrix.h"

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
            printf("%.4f\n", matrix[i*cols+j]);
        }
        printk("\n");
    }
    printk("\n\n");
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
int verify_matmul(float *mat1, float *mat2, int rows, int cols)
{
    int num_errors = 0;

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            if(mat1[i*cols+j]-mat2[i*cols+j] >= 0.01) num_errors++;
        }
    }

    return num_errors;
}

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
 * @brief push new node at the end of the queue
 * 
 * @param head 
 * @param newNode 
 */
void push_matmul(struct matmul *head, struct matmul *newNode)
{
    if (head == NULL) {
        head = newNode;
    } else {
        struct matmul *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
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


// void save_matmul(struct matmul *node)
// {
//     for(int i=0; i<node->mat1Rows; i++) {
//         for(int j=0; j<node->mat2Cols; j++) {
//             node->resultHW[i*node->mat2Cols+j] = resultHW[i*node->mat2Cols+j];
//         }
//     }

//     // int numErrors = verify_matmul(node->resultHW, node->resultSW, node->mat1Rows, node->mat2Cols);

//     // printf("Hardware matmul done with %d errors!\n", numErrors);
// }

/**
 * @brief multiply matrixes in node
 * 
 * @param calculatingNode 
 */
void perform_matmul(struct matmul *calculatingNode)
{
    multiply_mat_hw((int)calculatingNode->mat1, (int)calculatingNode->mat2, (int)calculatingNode->resultHW,
        calculatingNode->mat1Rows, calculatingNode->mat1Cols, calculatingNode->mat2Cols);
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