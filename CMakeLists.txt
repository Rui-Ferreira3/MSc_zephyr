cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_first_app)

# target_sources(app PRIVATE
#     nnUC0/main.c
#     nnUC0/header.h
#     nnUC0/common.h
#     nnUC0/data100.h
#     nnUC0/weights.h
#     nnUC0/nn.c
#     nnUC0/nn.h
#     nnUC0/multiply.c
#     nnUC0/multiply.h)

# target_sources(app PRIVATE
#     nnUC1/main.c
#     nnUC1/header.h
#     nnUC1/common.h
#     nnUC1/data100.h
#     nnUC1/weights.h
#     nnUC1/nn.c
#     nnUC1/nn.h
#     nnUC1/multiply.c
#     nnUC1/multiply.h)

target_sources(app PRIVATE
    nnUC2/main.c
    nnUC2/header.h
    nnUC2/common.h
    nnUC2/data100.h
    nnUC2/weights.h
    nnUC2/nn.c
    nnUC2/nn.h
    nnUC2/multiply.c
    nnUC2/multiply.h)

# target_sources(app PRIVATE
#     nnUC3/main.c
#     nnUC3/header.h
#     nnUC3/common.h
#     nnUC3/data100.h
#     nnUC3/weights.h
#     nnUC3/nn.c
#     nnUC3/nn.h
#     nnUC3/multiply.c
#     nnUC3/multiply.h)