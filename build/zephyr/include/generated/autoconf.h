#define CONFIG_GPIO 1
#define CONFIG_BOARD "veerwolf_nexys"
#define CONFIG_SOC "veerwolf"
#define CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC 50000000
#define CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME 1
#define CONFIG_RISCV_SOC_INTERRUPT_INIT 1
#define CONFIG_SWERV_PIC 1
#define CONFIG_NUM_IRQS 64
#define CONFIG_UART_NS16550 1
#define CONFIG_UART_NS16550_PORT_0 1
#define CONFIG_GPIO_MMIO32 1
#define CONFIG_RISCV_MACHINE_TIMER 1
#define CONFIG_RISCV_ROM_BASE_ADDR 0x00000000
#define CONFIG_RISCV_ROM_SIZE 0x0
#define CONFIG_GEN_ISR_TABLES 1
#define CONFIG_GEN_SW_ISR_TABLE 1
#define CONFIG_ISR_STACK_SIZE 2048
#define CONFIG_ATOMIC_OPERATIONS_C 1
#define CONFIG_RISCV_GENERIC_TOOLCHAIN 1
#define CONFIG_SYS_CLOCK_TICKS_PER_SEC 100
#define CONFIG_GEN_IRQ_START_VECTOR 0
#define CONFIG_MAIN_STACK_SIZE 1024
#define CONFIG_IDLE_STACK_SIZE 512
#define CONFIG_TEST_EXTRA_STACKSIZE 1024
#define CONFIG_KERNEL_ENTRY "__start"
#define CONFIG_GPIO_VEERWOLF 1
#define CONFIG_ZEPHYR_SWERVOLF_MODULE 1
#define CONFIG_ZEPHYR_CANOPENNODE_MODULE 1
#define CONFIG_ZEPHYR_HAL_NORDIC_MODULE 1
#define CONFIG_ZEPHYR_HAL_NXP_MODULE 1
#define CONFIG_ZEPHYR_LORAMAC_NODE_MODULE 1
#define CONFIG_ZEPHYR_LZ4_MODULE 1
#define CONFIG_ZEPHYR_MBEDTLS_MODULE 1
#define CONFIG_ZEPHYR_NANOPB_MODULE 1
#define CONFIG_ZEPHYR_SOF_MODULE 1
#define CONFIG_ZEPHYR_TFLITE_MICRO_MODULE 1
#define CONFIG_ZEPHYR_TRACERECORDER_MODULE 1
#define CONFIG_ZEPHYR_TRUSTED_FIRMWARE_M_MODULE 1
#define CONFIG_ROM_START_OFFSET 0x0
#define CONFIG_BOARD_VEERWOLF_NEXYS 1
#define CONFIG_SOC_RISCV32_VEERWOLF 1
#define CONFIG_ARCH "riscv32"
#define CONFIG_RISCV 1
#define CONFIG_ARCH_IS_SET 1
#define CONFIG_SRAM_SIZE 8192
#define CONFIG_SRAM_BASE_ADDRESS 0x0
#define CONFIG_ARCH_SW_ISR_TABLE_ALIGN 0
#define CONFIG_ARCH_HAS_THREAD_LOCAL_STORAGE 1
#define CONFIG_TOOLCHAIN_HAS_BUILTIN_FFS 1
#define CONFIG_MULTITHREADING 1
#define CONFIG_NUM_COOP_PRIORITIES 16
#define CONFIG_NUM_PREEMPT_PRIORITIES 15
#define CONFIG_MAIN_THREAD_PRIORITY 0
#define CONFIG_COOP_ENABLED 1
#define CONFIG_PREEMPT_ENABLED 1
#define CONFIG_PRIORITY_CEILING -127
#define CONFIG_NUM_METAIRQ_PRIORITIES 0
#define CONFIG_ERRNO 1
#define CONFIG_SCHED_DUMB 1
#define CONFIG_WAITQ_DUMB 1
#define CONFIG_DEVICE_HANDLE_PADDING 0
#define CONFIG_BOOT_BANNER 1
#define CONFIG_BOOT_DELAY 0
#define CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE 1024
#define CONFIG_SYSTEM_WORKQUEUE_PRIORITY -1
#define CONFIG_TIMESLICING 1
#define CONFIG_TIMESLICE_SIZE 0
#define CONFIG_TIMESLICE_PRIORITY 0
#define CONFIG_NUM_MBOX_ASYNC_MSGS 10
#define CONFIG_NUM_PIPE_ASYNC_MSGS 10
#define CONFIG_KERNEL_MEM_POOL 1
#define CONFIG_HEAP_MEM_POOL_SIZE 0
#define CONFIG_SYS_CLOCK_EXISTS 1
#define CONFIG_TIMEOUT_64BIT 1
#define CONFIG_KERNEL_INIT_PRIORITY_OBJECTS 30
#define CONFIG_KERNEL_INIT_PRIORITY_DEFAULT 40
#define CONFIG_KERNEL_INIT_PRIORITY_DEVICE 50
#define CONFIG_APPLICATION_INIT_PRIORITY 90
#define CONFIG_STACK_POINTER_RANDOM 0
#define CONFIG_MP_NUM_CPUS 1
#define CONFIG_HAS_DTS 1
#define CONFIG_HAS_DTS_GPIO 1
#define CONFIG_CONSOLE 1
#define CONFIG_CONSOLE_INPUT_MAX_LINE_LEN 128
#define CONFIG_CONSOLE_HAS_DRIVER 1
#define CONFIG_UART_CONSOLE 1
#define CONFIG_UART_CONSOLE_INIT_PRIORITY 60
#define CONFIG_SERIAL 1
#define CONFIG_SERIAL_HAS_DRIVER 1
#define CONFIG_SERIAL_SUPPORT_INTERRUPT 1
#define CONFIG_SERIAL_INIT_PRIORITY 50
#define CONFIG_UART_USE_RUNTIME_CONFIGURE 1
#define CONFIG_SYSTEM_CLOCK_INIT_PRIORITY 0
#define CONFIG_ENTROPY_GENERATOR 1
#define CONFIG_SUPPORT_MINIMAL_LIBC 1
#define CONFIG_NEWLIB_LIBC 1
#define CONFIG_HAS_NEWLIB_LIBC_NANO 1
#define CONFIG_NEWLIB_LIBC_NANO 1
#define CONFIG_NEWLIB_LIBC_MIN_REQUIRED_HEAP_SIZE 2048
#define CONFIG_NEWLIB_LIBC_FLOAT_PRINTF 1
#define CONFIG_STDOUT_CONSOLE 1
#define CONFIG_SYS_HEAP_ALLOC_LOOPS 3
#define CONFIG_CBPRINTF_COMPLETE 1
#define CONFIG_CBPRINTF_FULL_INTEGRAL 1
#define CONFIG_CBPRINTF_N_SPECIFIER 1
#define CONFIG_POSIX_MAX_FDS 4
#define CONFIG_MAX_TIMER_COUNT 5
#define CONFIG_PRINTK 1
#define CONFIG_EARLY_CONSOLE 1
#define CONFIG_ASSERT_VERBOSE 1
#define CONFIG_TEST_RANDOM_GENERATOR 1
#define CONFIG_TIMER_RANDOM_GENERATOR 1
#define CONFIG_TOOLCHAIN_ZEPHYR_0_13 1
#define CONFIG_LINKER_ORPHAN_SECTION_WARN 1
#define CONFIG_LD_LINKER_SCRIPT_SUPPORTED 1
#define CONFIG_LD_LINKER_TEMPLATE 1
#define CONFIG_LINKER_SORT_BY_ALIGNMENT 1
#define CONFIG_SRAM_OFFSET 0x0
#define CONFIG_LINKER_GENERIC_SECTIONS_PRESENT_AT_BOOT 1
#define CONFIG_SIZE_OPTIMIZATIONS 1
#define CONFIG_COMPILER_COLOR_DIAGNOSTICS 1
#define CONFIG_COMPILER_OPT ""
#define CONFIG_RUNTIME_ERROR_CHECKS 1
#define CONFIG_KERNEL_BIN_NAME "zephyr"
#define CONFIG_OUTPUT_STAT 1
#define CONFIG_OUTPUT_DISASSEMBLY 1
#define CONFIG_OUTPUT_PRINT_MEMORY_USAGE 1
#define CONFIG_BUILD_OUTPUT_BIN 1
#define CONFIG_COMPAT_INCLUDES 1
