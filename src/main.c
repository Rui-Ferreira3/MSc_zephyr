#include "header.h"

#define mat1Addr 0x02000000
#define mat2Addr 0x02000100
#define resultSwAddr 0x02000200
#define resultHwAddr 0x02000300

#define MY_DEV_IRQ  15       /* device uses IRQ 13 */
#define MY_DEV_PRIO  10       /* device uses interrupt priority 10 */


int mat1Rows=8, mat1Cols=8, mat2Rows=8, mat2Cols=8;
int accelFlag=0;

void my_isr_installer(void) {
    ARCH_IRQ_CONNECT(MY_DEV_IRQ, MY_DEV_PRIO, my_isr, NULL, 0);
    arch_irq_enable(MY_DEV_IRQ);
}

void my_isr(const void *arg) {
    *acceleratorGIER = 0;
    *acceleratorIP_IER = 0;
    printf("Inside my isr!\n");
    accelFlag=1;
    return;
}

int main()
{
    *acceleratorGIER = 1;
    *acceleratorIP_IER = 1;

    printf("Starting swerv irq!\n");
    my_isr_installer();

    create_mat(mat1Addr, mat1Rows, mat1Cols);
    create_mat(mat2Addr, mat2Rows, mat2Cols);

    printf("Starting matmul in hardware!\n");
    multiply_mat_hw(mat1Addr, mat2Addr, resultHwAddr, mat1Rows, mat1Cols, mat2Cols);
    // print_mat(resultHwAddr, mat1Rows, mat2Cols);
    while(accelFlag == 0) {}
    printf("Hardware matmul done!\n");
    multiply_mat_sw(mat1Addr, mat2Addr, resultSwAddr, mat1Rows, mat1Cols, mat2Cols);
    print_mat(resultSwAddr, mat1Rows, mat2Cols);

    int errors = verify_matmul(resultHwAddr, resultSwAddr, mat1Rows, mat2Cols);
    printf("Matmul done with %d errors!\n", errors);


    return 0;
}

// void reset_csr(int CSR_ADDR, int MASK) {
//     volatile uint32_t *csr = (uint32_t *)CSR_ADDR;
//     csr &= ~MASK;
// }

// void set_csr(int CSR_ADDR, int MASK) {
//     volatile uint32_t *csr = (uint32_t *)CSR_ADDR;
//     csr |= MASK;
// }

// void setup_interrupts() {
//     reset_csr(MIE_ADDR, MIE_EIE_MASK); // disable external interrupts

//     printf("1\n");

//     reset_CSR(PIC_MPICCFG_ADDR, MPICCFG_PRIORD_MASK); // configure priority order 0=lowest to 15=highest 

//     printf("2\n");

//     reset_csr(MEIPT_ADDR, MEIPT_PRITHRESH_MASK); // set priority threshold to 0

//     printf("4\n");

//     volatile int *meigwctrlS = (int *)(PIC_BASE_ADDRESS + 0x4004); // Address of meigwctrlS register
//     volatile int *meigwclrS = (int *)(PIC_BASE_ADDRESS + 0x5004); // Address of meigwclrS register
//     volatile int *meipIS = (int *)(PIC_BASE_ADDRESS + 0x0004); // Address of meipIS register
//     volatile int *meieS = (int *)(PIC_BASE_ADDRESS + 0x2004); // Address of meieS register
//     volatile int *addressTable = (int *)EXTERNAL_INTERRUPT_ADDRESS_TABLE;
//     for(int i=0; i<8; i++) {
//         set_csr(PIC_MEIGWCTRL_ADDR+4*i, 0x0002); // set type to edge triggered
//         reset_csr(PIC_MEIGWCTRL_ADDR+4*i, 0x0001); // set polarity to active-high
//         meigwclrS[i] = 0; // clear the IP bit in the meigwclrS register
//         meipIS[i] = 2; // set priority threshold to 2 for each external interrupt (bits 3 to 0)
//         meieS[i] |= 0x0001; // set inten (bit 0) to enable interrupts
//         addressTable[i] = &my_isr;
//     }

//     printf("5\n");

//     volatile int *mstatus = (int *)(CSR_BASE_ADDRESS + 0x300); // Address of mstatus register
//     *mstatus |= 1 << 3; // set bit mie (bit 3) of the mstatus CSR

//     printf("6\n");

//     *mie |= (1 << 11); // set bit miep (bit 11) of the mie CSR
// }