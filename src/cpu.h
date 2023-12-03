#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <vector>

#include "exception.h"
#include "interrupt.h"
#include "bus.h"

#define PAGE_SIZE 4096

#define MHARTID 0xf14
#define MSTATUS 0x300
#define MEDELEG 0x302
#define MIDELEG 0x303
#define MIE 0x304
#define MTVEC 0x305
#define MCOUNTEREN 0x306
#define MSCRATCH 0x340
#define MEPC 0x341
#define MCAUSE 0x342
#define MTVAL 0x343
#define MIP 0x344

#define MIP_SSIP (1 << 1)
#define MIP_MSIP (1 << 3)
#define MIP_STIP (1 << 5)
#define MIP_MTIP (1 << 7)
#define MIP_SEIP (1 << 9)
#define MIP_MEIP (1 << 11)

#define SSTATUS 0x100
#define SIE 0x104
#define STVEC 0x105
#define SSCRATCH 0x140
#define SEPC 0x141
#define SCAUSE 0x142
#define STVAL 0x143
#define SIP 0x144
#define SATP 0x180

enum Mode
{
    User = 0b00,
    Supervisor = 0b01,
    Machine = 0b11,
};

enum AccessType
{
    Instruction,
    Load,
    Store,
};

class Cpu
{
private:
    uint64_t registers[32];
    uint64_t csrs[4096];
    uint64_t pc;
    Mode mode;
    Bus bus;
    bool enable_paging;
    uint64_t page_table;

public:
    Cpu(std::vector<uint8_t> bytes, std::vector<uint8_t> disk_image);
    std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int nBytes);
    std::optional<Exception> store(uint64_t addr, int nBytes, uint64_t value);
    uint64_t load_csr(uint64_t addr);
    void store_csr(uint64_t addr, uint64_t value);
    std::pair<uint32_t, std::optional<Exception>> fetch();
    std::optional<Exception> execute(uint32_t instruction);
    void take_trap(Trap &trap, bool is_interrupt);
    std::optional<Interrupt> check_pending_interrupt();
    void disk_access();
    void update_paging(uint64_t csr_addr);
    std::pair<uint64_t, std::optional<Exception>> translate(uint64_t addr, AccessType access_type);
    uint64_t getPc() { return pc; };
    void setPc(uint64_t pc) { this->pc = pc; };
    Mode getMode() { return mode; };
    void setMode(Mode mode) { this->mode = mode; };
};

#endif
