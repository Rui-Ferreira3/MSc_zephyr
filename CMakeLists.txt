cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_first_app)

target_sources(app PRIVATE
    nnUC3/main.c
    nnUC3/header.h
    nnUC3/common.h
    nnUC3/data100.h
    nnUC3/weights.h
    nnUC3/nn.c
    nnUC3/nn.h
    nnUC3/multiply.c
    nnUC3/multiply.h)