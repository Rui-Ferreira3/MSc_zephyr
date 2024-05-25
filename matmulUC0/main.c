#include "header.h"

int main()
{
    printf("*** Starting matrix multiplication UC 0 ***\n\n");
    
    int addressOffset = MATRIX1_SIZE + MATRIX2_SIZE + RESULT_SW_SIZE + RESULT_POOL_SIZE;

    /* initialize the queue */
    printf("Saving %d matrix pairs to memory...\n", NUM_MULTIPLICATIONS);
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        create_mat(mat1Addr, MAT1ROWS, MAT1COLS);

        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        create_mat(mat2Addr, MAT1ROWS, MAT1COLS);
    }
    printf("%d saved to memory!\n", NUM_MULTIPLICATIONS);


    /* perform NUM_MULTIPLICATIONS in software */
#ifdef PERFORM_SW_ONLY
    printf("\nPerforming software matrix multiplication...\n");
    int start_sw_ms = k_uptime_get();
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        int resultAddr = mat2Addr + MATRIX2_SIZE;

        multiply_mat_sw(mat1Addr, mat2Addr, resultAddr, MAT1ROWS, MAT1COLS, MAT2COLS);
    }

    int finish_sw_ms = k_uptime_get();
    int time_sw = finish_sw_ms - start_sw_ms;

    printf("Completed software matrix multiplication!\n");
    printf("Execution time: %d ms\n", time_sw);
#endif //PERFORM_SW_ONLY


    /* perform NUM_MULTIPLICATIONS with pooling */
#ifdef PERFORM_POOLING
    printf("\nPerforming matrix multiplication with pooling...\n");
    int start_p_ms = k_uptime_get();
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        int resultAddr = mat2Addr + MATRIX2_SIZE + RESULT_SW_SIZE;

        multiply_mat_hw(mat1Addr, mat2Addr, resultAddr, MAT1ROWS, MAT1COLS, MAT2COLS);
    }

    int finish_p_ms = k_uptime_get();
    int time_p = finish_p_ms - start_p_ms;

    printf("Completed matrix multiplication with pooling!\n");
    printf("Execution time: %d ms\n", time_p);
#endif //PERFORM_POOLING


    /* check if software and hardware matmul match */
#if defined(PERFORM_SW_ONLY) && defined(PERFORM_POOLING)
    printf("\nChecking if software and hardware results match...\n");
    int numErrors = 0;
    for(int i=0; i<NUM_MULTIPLICATIONS; i++) {
        int mat1Addr = MEMORY_BASE_ADDRESS + i*addressOffset;
        int mat2Addr = mat1Addr + MATRIX1_SIZE;
        int resultSWAddr = mat2Addr + MATRIX2_SIZE;
        int resultPoolAddr = resultSWAddr + RESULT_SW_SIZE;

        numErrors += verify(resultSWAddr, resultPoolAddr, MAT1ROWS, MAT2COLS);
    }

    printf("\n%d operations done with %d errors!\n", NUM_MULTIPLICATIONS, numErrors);
#endif //PERFORM_SW_ONLY && PERFORM_POOLING

    printf("\n*** Exiting matrix multiplication UC 0 ***\n");

    return 0;
}