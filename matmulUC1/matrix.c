#include "matrix.h"

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

void init_matmul(struct matmul *node, int rows1, int cols1, int cols2)
{
    node->mat1 = (float *)k_malloc(rows1*cols1*sizeof(float));
    node->mat2 = (float *)k_malloc(cols1*cols2*sizeof(float));
    node->resultHW = (float *)k_malloc(rows1*cols2*sizeof(float));
    node->resultSW = (float *)k_malloc(rows1*cols2*sizeof(float));

    node->mat1Rows = rows1;
    node->mat1Cols = cols1;
    node->mat2Cols = cols2;

    node->next = NULL;
}

void fill_matmul(struct matmul *node)
{
    create_mat(node->mat1, node->mat1Rows, node->mat1Cols);
    create_mat(node->mat2, node->mat1Cols, node->mat2Cols);
    multiply_mat_sw(node->resultSW, node->mat1, node->mat2, node->mat1Rows, node->mat1Cols, node->mat2Cols);
}

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

void save_matmul(struct matmul *node)
{
    float *resultHW = (float *)RESULT_HW_ADDRESS;

    for(int i=0; i<node->mat1Rows; i++) {
        for(int j=0; j<node->mat2Cols; j++) {
            node->resultHW[i*node->mat2Cols+j] = resultHW[i*node->mat2Cols+j];
        }
    }

    int numErrors = verify_matmul(node->resultHW, node->resultSW, node->mat1Rows, node->mat2Cols);

    printf("Hardware matmul done with %d errors!\n", numErrors);
}

void perform_matmul(struct matmul *head, struct matmul **calculatingNode)
{
    struct matmul *node;
    float *mat1 = (float *)MAT1_ADDRESS;
    float *mat2 = (float *)MAT2_ADDRESS;

    pop2_matmul(head, &node);
    *calculatingNode = node;

    for(int i=0; i<node->mat1Rows; i++) {
        for(int j=0; j<node->mat1Cols; j++) {
            mat1[i*node->mat1Cols+j] = node->mat1[i*node->mat1Cols+j];
        }
    }

    for(int i=0; i<node->mat1Cols; i++) {
        for(int j=0; j<node->mat2Cols; j++) {
            mat2[i*node->mat2Cols+j] = node->mat2[i*node->mat2Cols+j];
        }
    }

    printf("Starting hardware matmul...\n");
    multiply_mat_hw(MAT1_ADDRESS, MAT2_ADDRESS, RESULT_HW_ADDRESS, node->mat1Rows, node->mat1Cols, node->mat2Cols);
}

void free_matmul(struct matmul *node)
{
    k_free(node->mat1);
    k_free(node->mat2);
    k_free(node->resultHW);
    k_free(node->resultSW);
    node->next = NULL;
    k_free(node);
}