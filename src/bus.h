#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include <vector>

#include "clint.h"
#include "exception.h"
#include "memory.h"
#include "plic.h"
#include "uart.h"
#include "virtio.h"

class Bus {
public:
    Memory memory;
    CLINT clint;
    PLIC plic;
    Uart uart;
    Virtio virtio;
    Bus(std::vector<uint8_t> bytes, std::vector<uint8_t> disk_image);
    std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int N);
    std::optional<Exception> store(uint64_t addr, int N, uint64_t value);
};

#endif
