cmake_minimum_required(VERSION 3.16)

project(riscv-emulator)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

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
