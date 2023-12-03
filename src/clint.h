#ifndef CLINT_H
#define CLINT_H

#include "device.h"

#define CLINT_BASE 0x2000000
#define CLINT_SIZE 0x10000
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000)
#define CLINT_MTIME (CLINT_BASE + 0xbff8)

class CLINT : public Device
{
private:
    uint64_t mtime;
    uint64_t mtimecmp;

public:
    CLINT();
    std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int nBytes);
    std::optional<Exception> store(uint64_t addr, int nBytes, uint64_t value);
};

#endif
