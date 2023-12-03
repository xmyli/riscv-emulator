#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <vector>
#include <optional>

#include "device.h"
#include "exception.h"

#define MEMORY_SIZE (1024 * 1024 * 128)
#define MEMORY_BASE 0x80000000

class Memory : public Device
{
private:
    std::vector<uint8_t> data;

public:
    Memory(std::vector<uint8_t> bytes);
    std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int nBytes);
    std::optional<Exception> store(uint64_t addr, int nBytes, uint64_t value);
};

#endif
