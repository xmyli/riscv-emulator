#include "interrupt.h"

#include <iostream>

Interrupt::Interrupt(InterruptType type) : type{type} {}

uint64_t Interrupt::get_code() {
    switch (type) {
    case InterruptType::UserSoftwareInterrupt:
        return 0;
    case InterruptType::SupervisorSoftwareInterrupt:
        return 1;
    case InterruptType::MachineSoftwareInterrupt:
        return 3;
    case InterruptType::UserTimerInterrupt:
        return 4;
    case InterruptType::SupervisorTimerInterrupt:
        return 5;
    case InterruptType::MachineTimerInterrupt:
        return 7;
    case InterruptType::UserExternalInterrupt:
        return 8;
    case InterruptType::SupervisorExternalInterrupt:
        return 9;
    case InterruptType::MachineExternalInterrupt:
        return 11;
    default:
        std::cerr << "error: illegal InterruptType" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
