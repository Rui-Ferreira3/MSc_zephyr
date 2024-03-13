cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_first_app)

target_sources(app PRIVATE
    matmulUC0/main.c
    matmulUC0/header.h
    matmulUC0/common.h
    matmulUC0/matrix.c
    matmulUC0/matrix.h
    matmulUC0/multiply.c
    matmulUC0/multiply.h)