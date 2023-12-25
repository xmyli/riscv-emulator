#include "plic.h"

#include <iostream>

PLIC::PLIC() : pending{0}, senable{0}, spriority{0}, sclaim{0} {}

std::pair<uint64_t, std::optional<Exception>> PLIC::load(uint64_t addr, int nBytes) {
    if (nBytes == 4) {
        switch (addr) {
        case PLIC_PENDING:
            return std::make_pair(pending, std::nullopt);
        case PLIC_SENABLE:
            return std::make_pair(senable, std::nullopt);
        case PLIC_SPRIORITY:
            return std::make_pair(spriority, std::nullopt);
        case PLIC_SCLAIM:
            return std::make_pair(sclaim, std::nullopt);
        default:
            return std::make_pair(0, std::nullopt);
        }
    }
    return std::make_pair(0, Exception(ExceptionType::LoadAccessFault));
}

std::optional<Exception> PLIC::store(uint64_t addr, int nBytes, uint64_t value) {
    if (nBytes == 4) {
        switch (addr) {
        case PLIC_PENDING:
            pending = value;
            return std::nullopt;
        case PLIC_SENABLE:
            senable = value;
            return std::nullopt;
        case PLIC_SPRIORITY:
            spriority = value;
            return std::nullopt;
        case PLIC_SCLAIM:
            sclaim = value;
            return std::nullopt;
        default:
            return std::nullopt;
        }
    }
    return Exception(ExceptionType::StoreAMOAccessFault);
}
