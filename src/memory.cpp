#include <iostream>

#include "memory.h"

Memory::Memory(std::vector<uint8_t> bytes) : data{bytes} {
    data.resize(MEMORY_SIZE);
}

std::pair<uint64_t, std::optional<Exception>> Memory::load(uint64_t addr, int nBytes) {
    uint64_t result = 0;
    for (int i = 0; i < nBytes; i++) {
        if (addr - MEMORY_BASE + i >= data.size()) {
            return std::make_pair(0, Exception(ExceptionType::LoadAccessFault));
        }
        result |= (uint64_t)data[addr - MEMORY_BASE + i] << i * 8;
    }
    return std::make_pair(result, std::nullopt);
}

std::optional<Exception> Memory::store(uint64_t addr, int nBytes, uint64_t value) {
    for (int i = 0; i < nBytes; i++) {
        if (addr - MEMORY_BASE + i >= data.size()) {
            return Exception(ExceptionType::StoreAMOAccessFault);
        }
        uint8_t byteToStore = (value & ((uint64_t)0xff << i * 8)) >> i * 8;
        data[addr - MEMORY_BASE + i] = byteToStore;
    }
    return std::nullopt;
}
