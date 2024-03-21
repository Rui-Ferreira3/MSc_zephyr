cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_first_app)

target_sources(app PRIVATE
    matmulUC3/main.c
    matmulUC3/header.h
    matmulUC3/common.h
    matmulUC3/matrix.c
    matmulUC3/matrix.h
    matmulUC3/multiply.c
    matmulUC3/multiply.h)