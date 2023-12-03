#ifndef TRAP_H
#define TRAP_H

#include <cstdint>

class Trap
{
public:
    virtual uint64_t get_code() = 0;
};

#endif
