#!/bin/bash

make CROSS_COMPILE=riscv64-linux-gnu- clean
make FUZZING=1 CROSS_COMPILE=riscv64-linux-gnu- PLATFORM_RISCV_XLEN=64 PLATFORM=generic # V=1
