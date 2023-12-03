#ifndef PLIC_H
#define PLIC_H

#include "device.h"

#define PLIC_BASE 0xc000000
#define PLIC_SIZE 0x4000000
#define PLIC_PENDING (PLIC_BASE + 0x1000)
#define PLIC_SENABLE (PLIC_BASE + 0x2080)
#define PLIC_SPRIORITY (PLIC_BASE + 0x201000)
#define PLIC_SCLAIM (PLIC_BASE + 0x201004)

class PLIC : public Device
{
private:
    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;

public:
    PLIC();
    std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int nBytes);
    std::optional<Exception> store(uint64_t addr, int nBytes, uint64_t value);
};

#endif
