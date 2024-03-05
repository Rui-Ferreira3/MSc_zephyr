cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_first_app)

target_sources(app PRIVATE
    matmulUC1/main.c
    matmulUC1/matrix.c
    matmulUC1/multiply.c)