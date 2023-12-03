#include "clint.h"

#include <iostream>

CLINT::CLINT() : mtime{0}, mtimecmp{0} {}

std::pair<uint64_t, std::optional<Exception>> CLINT::load(uint64_t addr, int nBytes)
{
    if (nBytes == 8)
    {
        if (addr == CLINT_MTIMECMP)
        {
            return std::make_pair(mtimecmp, std::nullopt);
        }
        else if (addr == CLINT_MTIME)
        {
            return std::make_pair(mtime, std::nullopt);
        }
        return std::make_pair(0, std::nullopt);
    }
    return std::make_pair(0, Exception(ExceptionType::LoadAccessFault));
}

std::optional<Exception> CLINT::store(uint64_t addr, int nBytes, uint64_t value)
{
    if (nBytes == 8)
    {
        if (addr == CLINT_MTIMECMP)
        {
            mtimecmp = value;
            return std::nullopt;
        }
        else if (addr == CLINT_MTIME)
        {
            mtime = value;
            return std::nullopt;
        }
        return std::nullopt;
    }
    return Exception(ExceptionType::StoreAMOAccessFault);
}
