#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "trap.h"

enum ExceptionType
{
    InstructionAddressMisaligned,
    InstructionAccessFault,
    IllegalInstruction,
    Breakpoint,
    LoadAddressMisaligned,
    LoadAccessFault,
    StoreAMOAddressMisaligned,
    StoreAMOAccessFault,
    EnvironmentCallFromUMode,
    EnvironmentCallFromSMode,
    EnvironmentCallFromMMode,
    InstructionPageFault,
    LoadPageFault,
    StoreAMOPageFault,
};

class Exception : public Trap
{
private:
    ExceptionType type;

public:
    Exception(ExceptionType type);
    uint64_t get_code();
    bool is_fatal();
};

#endif
