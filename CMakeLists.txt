cmake_minimum_required(VERSION 3.16)

project(riscv-emulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(
    riscv-emulator

    src/trap.h
    src/exception.h
    src/interrupt.h
    src/device.h
    src/memory.h
    src/clint.h
    src/plic.h
    src/uart.h
    src/virtio.h
    src/bus.h
    src/cpu.h

    src/exception.cpp
    src/interrupt.cpp
    src/memory.cpp
    src/clint.cpp
    src/plic.cpp
    src/uart.cpp
    src/virtio.cpp
    src/bus.cpp
    src/cpu.cpp
    src/main.cpp
)
