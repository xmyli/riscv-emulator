#ifndef DEVICE_H
#define DEVICE_H

#include <optional>
#include <utility>

#include "exception.h"

class Device
{
public:
    virtual std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int nBytes) = 0;
    virtual std::optional<Exception> store(uint64_t addr, int nBytes, uint64_t value) = 0;
};

#endif
