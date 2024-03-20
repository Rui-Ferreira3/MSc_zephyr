cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_first_app)

target_sources(app PRIVATE
    matmulUC2/main.c
    matmulUC2/header.h
    matmulUC2/common.h
    matmulUC2/matrix.c
    matmulUC2/matrix.h
    matmulUC2/multiply.c
    matmulUC2/multiply.h)