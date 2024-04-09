cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_first_app)

target_sources(app PRIVATE
    nnUC2/main.c
    nnUC2/header.h
    nnUC2/common.h
    nnUC2/data.h
    nnUC2/weights.h
    nnUC2/nn.c
    nnUC2/nn.h
    nnUC2/multiply.c
    nnUC2/multiply.h)