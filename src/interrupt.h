#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "trap.h"

enum InterruptType {
    UserSoftwareInterrupt,
    SupervisorSoftwareInterrupt,
    MachineSoftwareInterrupt,
    UserTimerInterrupt,
    SupervisorTimerInterrupt,
    MachineTimerInterrupt,
    UserExternalInterrupt,
    SupervisorExternalInterrupt,
    MachineExternalInterrupt,
};

class Interrupt : public Trap {
private:
    InterruptType type;

public:
    Interrupt(InterruptType type);
    uint64_t get_code();
};

#endif
