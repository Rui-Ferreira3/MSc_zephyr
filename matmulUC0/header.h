#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "common.h"
#include "matrix.h"
#include "multiply.h"

// #define PERFORM_SW_ONLY // Uncomment to perform software only
#define PERFORM_POOLING // Uncomment to perform hardware acceleration with pooling

#endif //HEADER_H