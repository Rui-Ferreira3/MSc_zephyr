#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <random/rand32.h>

#include "common.h"
#include "data.h"
#include "weights.h"
#include "nn.h"
#include "multiply.h"

void get_digit(int num);

#endif //HEADER_H