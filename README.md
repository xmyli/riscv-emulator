# RISC-V Emulator

RISC-V emulator implementing the RV64I base ISA, the RVZicsr extensions, and parts of the RV64M and RV64A extensions.

![Demo](https://github.com/xmyli/riscv-emulator/blob/main/demo.png)

## Usage

This projects includes a CMakeLists.txt file for building using cmake.

1. Clone the repository.
   ```
   git clone https://github.com/xmyli/riscv-emulator/ && cd riscv-emulator
   ```
2. Set up the build directory and compile.
   ```
   cmake -S . -B build && cmake --build build
   ```
3. Run the executable using the provided binary files as parameters.
   ```
   ./build/riscv-emulator ./xv6-kernel.bin ./xv6-fs.img
   ```
