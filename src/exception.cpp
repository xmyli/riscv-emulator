#include "exception.h"

#include <iostream>

Exception::Exception(ExceptionType type) : type{type} {}

uint64_t Exception::get_code()
{
    switch (type)
    {
    case ExceptionType::InstructionAddressMisaligned:
        return 0;
    case ExceptionType::InstructionAccessFault:
        return 1;
    case ExceptionType::IllegalInstruction:
        return 2;
    case ExceptionType::Breakpoint:
        return 3;
    case ExceptionType::LoadAddressMisaligned:
        return 4;
    case ExceptionType::LoadAccessFault:
        return 5;
    case ExceptionType::StoreAMOAddressMisaligned:
        return 6;
    case ExceptionType::StoreAMOAccessFault:
        return 7;
    case ExceptionType::EnvironmentCallFromUMode:
        return 8;
    case ExceptionType::EnvironmentCallFromSMode:
        return 9;
    case ExceptionType::EnvironmentCallFromMMode:
        return 11;
    case ExceptionType::InstructionPageFault:
        return 12;
    case ExceptionType::LoadPageFault:
        return 13;
    case ExceptionType::StoreAMOPageFault:
        return 15;
    default:
        std::cerr << "error: illegal ExceptionType" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

bool Exception::is_fatal()
{
    switch (type)
    {
    case ExceptionType::InstructionAddressMisaligned:
        return true;
    case ExceptionType::InstructionAccessFault:
        return true;
    case ExceptionType::LoadAccessFault:
        return true;
    case ExceptionType::StoreAMOAddressMisaligned:
        return true;
    case ExceptionType::StoreAMOAccessFault:
        return true;
    default:
        return false;
    }
}
