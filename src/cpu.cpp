#include <cstdint>
#include <iostream>
#include <vector>

#include "cpu.h"

Cpu::Cpu(std::vector<uint8_t> bytes, std::vector<uint8_t> disk_image) : registers{0},
                                                                        csrs{0},
                                                                        pc{MEMORY_BASE},
                                                                        mode{Mode::Machine},
                                                                        bus{Bus(bytes, disk_image)},
                                                                        enable_paging{false},
                                                                        page_table{0} {
    registers[2] = MEMORY_BASE + MEMORY_SIZE;
}

std::pair<uint64_t, std::optional<Exception>> Cpu::load(uint64_t addr, int nBytes) {
    auto [p_addr, err] = translate(addr, AccessType::Load);
    if (err.has_value()) {
        return std::make_pair(p_addr, err);
    }
    auto [data, errr] = bus.load(p_addr, nBytes);
    return std::make_pair(data, errr);
}

std::optional<Exception> Cpu::store(uint64_t addr, int nBytes, uint64_t value) {
    auto [p_addr, err] = translate(addr, AccessType::Store);
    if (err.has_value()) {
        return err;
    }
    return bus.store(p_addr, nBytes, value);
}

uint64_t Cpu::load_csr(uint64_t addr) {
    switch (addr) {
    case SIE:
        return csrs[MIE] & csrs[MIDELEG];
    default:
        return csrs[addr];
    }
}

void Cpu::store_csr(uint64_t addr, uint64_t value) {
    switch (addr) {
    case SIE:
        csrs[MIE] = (csrs[MIE] & ~csrs[MIDELEG]) | (value & csrs[MIDELEG]);
        return;
    default:
        csrs[addr] = value;
        return;
    }
}

std::pair<uint32_t, std::optional<Exception>> Cpu::fetch() {
    auto [p_pc, translate_err] = translate(pc, AccessType::Instruction);
    if (translate_err.has_value()) {
        return std::make_pair(0, translate_err);
    }
    auto [instruction, load_err] = bus.load(p_pc, 4);
    if (load_err.has_value()) {
        return std::make_pair(0, Exception(ExceptionType::InstructionAccessFault));
    }
    return std::make_pair(instruction, std::nullopt);
}

std::optional<Exception> Cpu::execute(uint32_t instruction) {
    registers[0] = 0;

    auto opcode = instruction & 0x0000007f;
    auto rd = (instruction & 0x00000f80) >> 7;
    auto rs1 = (instruction & 0x000f8000) >> 15;
    auto rs2 = (instruction & 0x01f00000) >> 20;
    auto funct3 = (instruction & 0x00007000) >> 12;
    auto funct7 = (instruction & 0xfe000000) >> 25;

    switch (opcode) {
    case 0x3: {
        uint64_t imm = (int64_t)(int32_t)instruction >> 20;
        uint64_t addr = registers[rs1] + imm;

        switch (funct3) {
        case 0x0: {
            // lb
            auto [data, err] = load(addr, 1);
            if (err.has_value()) {
                return err;
            }
            registers[rd] = (int64_t)(int8_t)data;
            return std::nullopt;
        }
        case 0x1: {
            // lh
            auto [data, err] = load(addr, 2);
            if (err.has_value()) {
                return err;
            }
            registers[rd] = (int64_t)(int16_t)data;
            return std::nullopt;
        }
        case 0x2: {
            // lw
            auto [data, err] = load(addr, 4);
            if (err.has_value()) {
                return err;
            }
            registers[rd] = (int64_t)(int32_t)data;
            return std::nullopt;
        }
        case 0x3: {
            // ld
            auto [data, err] = load(addr, 8);
            if (err.has_value()) {
                return err;
            }
            registers[rd] = data;
            return std::nullopt;
        }
        case 0x4: {
            // lbu
            auto [data, err] = load(addr, 1);
            if (err.has_value()) {
                return err;
            }
            registers[rd] = data;
            return std::nullopt;
        }
        case 0x5: {
            // lhu
            auto [data, err] = load(addr, 2);
            if (err.has_value()) {
                return err;
            }
            registers[rd] = data;
            return std::nullopt;
        }
        case 0x6: {
            // lwu
            auto [data, err] = load(addr, 4);
            if (err.has_value()) {
                return err;
            }
            registers[rd] = data;
            return std::nullopt;
        }
        default:
            std::cout << "IllegalInstruction(1): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0xf: {
        switch (funct3) {
        case 0x0:
            // fence
            return std::nullopt;
        default:
            std::cout << "IllegalInstruction(2): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x13: {
        uint64_t imm = (int64_t)(int32_t)(instruction & 0xfff00000) >> 20;
        uint32_t shamt = imm & 0x3f;

        switch (funct3) {
        case 0x0:
            // addi
            registers[rd] = registers[rs1] + imm;
            return std::nullopt;
        case 0x1:
            // slli
            registers[rd] = registers[rs1] << shamt;
            return std::nullopt;
        case 0x2:
            // slti
            registers[rd] = (int64_t)registers[rs1] < (int64_t)imm ? 1 : 0;
            return std::nullopt;
        case 0x3:
            // sltiu
            registers[rd] = registers[rs1] < imm ? 1 : 0;
            return std::nullopt;
        case 0x4:
            // xori
            registers[rd] = registers[rs1] ^ imm;
            return std::nullopt;
        case 0x5:
            if (funct7 >> 1 == 0x00) {
                // srli
                registers[rd] = registers[rs1] >> shamt;
                return std::nullopt;
            } else if (funct7 >> 1 == 0x10) {
                // srai
                registers[rd] = (uint64_t)((int64_t)registers[rs1] >> shamt);
                return std::nullopt;
            } else {
                std::cout << "IllegalInstruction(3): " << instruction << std::endl;
                return Exception(ExceptionType::IllegalInstruction);
            }
        case 0x6:
            // ori
            registers[rd] = registers[rs1] | imm;
            return std::nullopt;
        case 0x7:
            // andi
            registers[rd] = registers[rs1] & imm;
            return std::nullopt;
        default:
            std::cout << "IllegalInstruction(4): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x17: {
        // auip
        uint64_t imm = (int64_t)(int32_t)(instruction & 0xfffff000);
        registers[rd] = pc + imm - 4;
        return std::nullopt;
    }
    case 0x1b: {
        uint64_t imm = (int64_t)(int32_t)instruction >> 20;
        uint32_t shamt = imm & 0x1f;

        switch (funct3) {
        case 0x0:
            // addiw
            registers[rd] = (int64_t)(int32_t)(registers[rs1] + imm);
            return std::nullopt;
        case 0x1:
            // slliw
            registers[rd] = (int64_t)(int32_t)(registers[rs1] << shamt);
            return std::nullopt;
        case 0x5:
            if (funct7 == 0x00) {
                // srliw
                registers[rd] = (int64_t)(int32_t)((uint32_t)registers[rs1] >> shamt);
                return std::nullopt;
            } else if (funct7 == 0x20) {
                // sraiw
                registers[rd] = (int64_t)((int32_t)registers[rs1] >> shamt);
                return std::nullopt;
            } else {
                std::cout << "IllegalInstruction(5): " << instruction << std::endl;
                return Exception(ExceptionType::IllegalInstruction);
            }
        default:
            std::cout << "IllegalInstruction(6): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x23: {
        uint64_t imm = (uint64_t)((int64_t)(int32_t)(instruction & 0xfe000000) >> 20) | ((instruction >> 7) & 0x1f);
        uint64_t addr = registers[rs1] + imm;

        switch (funct3) {
        case 0x0: {
            // sb
            auto err = store(addr, 1, registers[rs2]);
            if (err.has_value()) {
                return err;
            }
            return std::nullopt;
        }
        case 0x1: {
            // sh
            auto err = store(addr, 2, registers[rs2]);
            if (err.has_value()) {
                return err;
            }
            return std::nullopt;
        }
        case 0x2: {
            // sw
            auto err = store(addr, 4, registers[rs2]);
            if (err.has_value()) {
                return err;
            }
            return std::nullopt;
        }
        case 0x3: {
            // sd
            auto err = store(addr, 8, registers[rs2]);
            if (err.has_value()) {
                return err;
            }
            return std::nullopt;
        }
        default:
            std::cout << "IllegalInstruction(7): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x2f: {
        auto funct5 = (funct7 & 0b1111100) >> 2;
        switch (funct3) {
        case 0x2:
            switch (funct5) {
            case 0x00: {
                // amoadd.w
                auto [temp, ld_err] = load(registers[rs1], 4);
                if (ld_err.has_value()) {
                    return ld_err;
                }
                auto st_err = store(registers[rs1], 4, temp + registers[rs2]);
                if (st_err.has_value()) {
                    return st_err;
                }
                registers[rd] = temp;
                return std::nullopt;
            }
            case 0x01: {
                // amoswap.w
                auto [temp, ld_err] = load(registers[rs1], 4);
                if (ld_err.has_value()) {
                    return ld_err;
                }
                auto st_err = store(registers[rs1], 4, registers[rs2]);
                if (st_err.has_value()) {
                    return st_err;
                }
                registers[rd] = temp;
                return std::nullopt;
            }
            default:
                std::cout << "IllegalInstruction(8): " << instruction << std::endl;
                return Exception(ExceptionType::IllegalInstruction);
            }
        case 0x3:
            switch (funct5) {
            case 0x00: {
                // amoadd.d
                auto [temp, ld_err] = load(registers[rs1], 8);
                if (ld_err.has_value()) {
                    return ld_err;
                }
                auto st_err = store(registers[rs1], 8, temp + registers[rs2]);
                if (st_err.has_value()) {
                    return st_err;
                }
                registers[rd] = temp;
                return std::nullopt;
            }
            case 0x01: {
                // amoswap.d
                auto [temp, ld_err] = load(registers[rs1], 8);
                if (ld_err.has_value()) {
                    return ld_err;
                }
                auto st_err = store(registers[rs1], 8, registers[rs2]);
                if (st_err.has_value()) {
                    return st_err;
                }
                registers[rd] = temp;
                return std::nullopt;
            }
            default:
                std::cout << "IllegalInstruction(9): " << instruction << std::endl;
                return Exception(ExceptionType::IllegalInstruction);
            }
        default:
            std::cout << "IllegalInstruction(10): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x33: {
        uint32_t shamt = (uint64_t)(registers[rs2] & 0x3f);

        switch (funct3) {
        case 0x0:
            if (funct7 == 0x00) {
                // add
                registers[rd] = registers[rs1] + registers[rs2];
                return std::nullopt;
            } else if (funct7 == 0x01) {
                // mul
                registers[rd] = registers[rs1] * registers[rs2];
                return std::nullopt;
            } else if (funct7 == 0x20) {
                // sub
                registers[rd] = registers[rs1] - registers[rs2];
                return std::nullopt;
            } else {
                std::cout << "IllegalInstruction(11): " << instruction << std::endl;
                return Exception(ExceptionType::IllegalInstruction);
            }
        case 0x1:
            // sll
            registers[rd] = registers[rs1] << shamt;
            return std::nullopt;
        case 0x2:
            // slt
            registers[rd] = (int64_t)registers[rs1] < (int64_t)registers[rs2] ? 1 : 0;
            return std::nullopt;
        case 0x3:
            // sltu
            registers[rd] = registers[rs1] < registers[rs2] ? 1 : 0;
            return std::nullopt;
        case 0x4:
            // xor
            registers[rd] = registers[rs1] ^ registers[rs2];
            return std::nullopt;
        case 0x5:
            if (funct7 == 0x00) {
                // srl
                registers[rd] = registers[rs1] >> shamt;
                return std::nullopt;
            } else if (funct7 == 0x20) {
                // sra
                registers[rd] = (int64_t)registers[rs1] >> shamt;
                return std::nullopt;
            }
            std::cout << "IllegalInstruction(12): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        case 0x6:
            // or
            registers[rd] = registers[rs1] | registers[rs2];
            return std::nullopt;
        case 0x7:
            // and
            registers[rd] = registers[rs1] & registers[rs2];
            return std::nullopt;
        default:
            std::cout << "IllegalInstruction(13): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x37: {
        // lu
        registers[rd] = (int64_t)(int32_t)(instruction & 0xfffff000);
        return std::nullopt;
    }
    case 0x3b: {
        uint32_t shamt = registers[rs2] & 0x1f;

        switch (funct3) {
        case 0x0:
            if (funct7 == 0x00) {
                // addw
                registers[rd] = (int64_t)(int32_t)(registers[rs1] + registers[rs2]);
                return std::nullopt;
            } else if (funct7 == 0x20) {
                // subw
                registers[rd] = (int32_t)(registers[rs1] - registers[rs2]);
                return std::nullopt;
            }
            std::cout << "IllegalInstruction(14): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        case 0x1:
            // sllw
            registers[rd] = (int32_t)((uint32_t)registers[rs1] << shamt);
            return std::nullopt;
        case 0x5:
            if (funct7 == 0x00) {
                // srlw
                registers[rd] = (int32_t)((uint32_t)registers[rs1] >> shamt);
                return std::nullopt;
            } else if (funct7 == 0x01) {
                // divu
                if (registers[rs2] == 0) {
                    registers[rd] = 0xffffffffffffffff;
                } else {
                    auto dividend = registers[rs1];
                    auto divisor = registers[rs2];
                    registers[rd] = dividend / divisor;
                }
                return std::nullopt;
            } else if (funct7 == 0x20) {
                // sraw
                registers[rd] = (int32_t)registers[rs1] >> (int32_t)shamt;
                return std::nullopt;
            }
            std::cout << "IllegalInstruction(15): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        case 0x7: {
            // remuw
            if (registers[rs2] == 0) {
                registers[rd] = registers[rs1];
            } else {
                uint32_t dividend = registers[rs1];
                uint32_t divisor = registers[rs2];
                registers[rd] = (int32_t)(dividend % divisor);
            }
            return std::nullopt;
        }
        default:
            std::cout << "IllegalInstruction(16): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x63: {
        uint64_t imm = (uint64_t)((int64_t)(int32_t)(instruction & 0x80000000) >> 19) | ((instruction & 0x80) << 4) | ((instruction >> 20) & 0x7e0) | ((instruction >> 7) & 0x1e);

        switch (funct3) {
        case 0x0:
            // beq
            if (registers[rs1] == registers[rs2]) {
                pc = pc + imm - 4;
            }
            return std::nullopt;
        case 0x1:
            // bne
            if (registers[rs1] != registers[rs2]) {
                pc = pc + imm - 4;
            }
            return std::nullopt;
        case 0x4:
            // blt
            if ((int64_t)registers[rs1] < (int64_t)registers[rs2]) {
                pc = pc + imm - 4;
            }
            return std::nullopt;
        case 0x5:
            // bge
            if ((int64_t)registers[rs1] >= (int64_t)registers[rs2]) {
                pc = pc + imm - 4;
            }
            return std::nullopt;
        case 0x6:
            // bltu
            if (registers[rs1] < registers[rs2]) {
                pc = pc + imm - 4;
            }
            return std::nullopt;
        case 0x7:
            // bgeu
            if (registers[rs1] >= registers[rs2]) {
                pc = pc + imm - 4;
            }
            return std::nullopt;
        default:
            std::cout << "IllegalInstruction(17): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    case 0x67: {
        // jalr
        auto temp = pc;
        uint64_t imm = (int64_t)(int32_t)(instruction & 0xfff00000) >> 20;
        pc = (registers[rs1] + imm) & ~1;
        registers[rd] = temp;
        return std::nullopt;
    }
    case 0x6f: {
        // jal
        registers[rd] = pc;
        uint64_t imm = (uint64_t)((int64_t)(int32_t)(instruction & 0x80000000) >> 11) | (instruction & 0xff000) | ((instruction >> 9) & 0x800) | ((instruction >> 20) & 0x7fe);
        pc = pc + imm - 4;
        return std::nullopt;
    }
    case 0x73: {
        uint64_t csr_addr = (instruction & 0xfff00000) >> 20;

        switch (funct3) {
        case 0x0: {
            if (rs2 == 0x0 && funct7 == 0x0) {
                // ecall
                switch (mode) {
                case Mode::User:
                    return Exception(ExceptionType::EnvironmentCallFromUMode);
                case Mode::Supervisor:
                    return Exception(ExceptionType::EnvironmentCallFromSMode);
                case Mode::Machine:
                    return Exception(ExceptionType::EnvironmentCallFromMMode);
                }
            } else if (rs2 == 0x1 && funct7 == 0x0) {
                // ebreak
                return Exception(ExceptionType::Breakpoint);
            } else if (rs2 == 0x2) {
                if (funct7 == 0x8) {
                    // sret
                    pc = load_csr(SEPC);

                    auto spp = load_csr(SSTATUS) >> 8 & 1;
                    mode = spp == 1 ? Mode::Supervisor : Mode::User;

                    auto spie = (load_csr(SSTATUS) >> 5) & 1;
                    auto sie_set = load_csr(SSTATUS) | (1 << 1);
                    auto sie_unset = load_csr(SSTATUS) & ~(1 << 1);

                    store_csr(SSTATUS, spie == 1 ? sie_set : sie_unset);
                    store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 5));
                    store_csr(SSTATUS, load_csr(SSTATUS) & ~(1 << 8));

                    return std::nullopt;
                } else if (funct7 == 0x18) {
                    // mret
                    pc = load_csr(MEPC);

                    auto mpp = (load_csr(MSTATUS) >> 11) & 0b11;
                    if (mpp == 2) {
                        mode = Mode::Machine;
                    } else if (mpp == 1) {
                        mode = Mode::Supervisor;
                    } else {
                        mode = Mode::User;
                    }

                    auto mpie = (load_csr(MSTATUS) >> 7) & 1;
                    auto mie_set = load_csr(MSTATUS) | (1 << 3);
                    auto mie_unset = load_csr(MSTATUS) & ~(1 << 3);

                    store_csr(MSTATUS, mpie == 1 ? mie_set : mie_unset);
                    store_csr(MSTATUS, load_csr(MSTATUS) | (1 << 7));
                    store_csr(MSTATUS, load_csr(MSTATUS) & ~(0b11 << 11));

                    return std::nullopt;
                } else {
                    std::cout << "IllegalInstruction(18): " << instruction << std::endl;
                    return Exception(ExceptionType::IllegalInstruction);
                }
            } else if (funct7 == 0x9) {
                // sfence.vma
                return std::nullopt;
            } else {
                std::cout << "IllegalInstruction(19): " << instruction << std::endl;
                return Exception(ExceptionType::IllegalInstruction);
            }
        }
        case 0x1: {
            // csrrw
            auto temp = load_csr(csr_addr);
            store_csr(csr_addr, registers[rs1]);
            registers[rd] = temp;
            update_paging(csr_addr);
            return std::nullopt;
        }
        case 0x2: {
            // csrrs
            auto temp = load_csr(csr_addr);
            store_csr(csr_addr, temp | registers[rs1]);
            registers[rd] = temp;
            update_paging(csr_addr);
            return std::nullopt;
        }
        case 0x3: {
            // csrrc
            auto temp = load_csr(csr_addr);
            store_csr(csr_addr, temp & ~registers[rs1]);
            registers[rd] = temp;
            update_paging(csr_addr);
            return std::nullopt;
        }
        case 0x5: {
            // csrrwi
            uint64_t zimm = rs1;
            registers[rd] = load_csr(csr_addr);
            store_csr(csr_addr, zimm);
            update_paging(csr_addr);
            return std::nullopt;
        }
        case 0x6: {
            // csrrsi
            uint64_t zimm = rs1;
            auto temp = load_csr(csr_addr);
            store_csr(csr_addr, temp | zimm);
            registers[rd] = temp;
            update_paging(csr_addr);
            return std::nullopt;
        }
        case 0x7: {
            // csrrci
            uint64_t zimm = rs1;
            auto temp = load_csr(csr_addr);
            store_csr(csr_addr, temp & ~zimm);
            registers[rd] = temp;
            update_paging(csr_addr);
            return std::nullopt;
        }
        default:
            std::cout << "IllegalInstruction(20): " << instruction << std::endl;
            return Exception(ExceptionType::IllegalInstruction);
        }
    }
    default:
        std::cout << "IllegalInstruction(21): " << instruction << std::endl;
        return Exception(ExceptionType::IllegalInstruction);
    }
}

void Cpu::take_trap(Trap &trap, bool is_interrupt) {
    uint64_t exception_pc = pc - 4;
    Mode previous_mode = mode;

    auto cause = trap.get_code();

    if (is_interrupt) {
        cause = ((uint64_t)1 << 63) | cause;
    }

    if (previous_mode <= Mode::Supervisor && ((load_csr(MEDELEG) >> (uint32_t)cause) & 1) != 0) {
        setMode(Mode::Supervisor);

        if (is_interrupt) {
            auto vector = (load_csr(STVEC) & 1) == 1 ? 4 * cause : 0;
            setPc((load_csr(STVEC) & ~1) + vector);
        } else {
            setPc(load_csr(STVEC) & ~1);
        }

        store_csr(SEPC, exception_pc & ~1);
        store_csr(SCAUSE, cause);
        store_csr(STVAL, 0);

        store_csr(SSTATUS, ((load_csr(SSTATUS) >> 1) & 1) == 1 ? load_csr(SSTATUS) | (1 << 5) : load_csr(SSTATUS) & ~(1 << 5));
        store_csr(SSTATUS, load_csr(SSTATUS) & ~(1 << 1));

        if (previous_mode == Mode::User) {
            store_csr(SSTATUS, load_csr(SSTATUS) & ~(1 << 8));
        } else {
            store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 8));
        }
    } else {
        setMode(Mode::Machine);

        if (is_interrupt) {
            auto vector = (load_csr(MTVEC) & 1) == 1 ? 4 * cause : 0;
            setPc((load_csr(MTVEC) & ~1) + vector);
        } else {
            setPc(load_csr(MTVEC) & ~1);
        }

        store_csr(MEPC, exception_pc & ~1);
        store_csr(MCAUSE, cause);
        store_csr(MTVAL, 0);
        store_csr(MSTATUS, ((load_csr(MSTATUS) >> 3) & 1) == 1 ? load_csr(MSTATUS) | (1 << 7) : load_csr(MSTATUS) & ~(1 << 7));
        store_csr(MSTATUS, load_csr(MSTATUS) & ~(1 << 3));
        store_csr(MSTATUS, load_csr(MSTATUS) & ~(0b11 << 11));
    }
}

std::optional<Interrupt> Cpu::check_pending_interrupt() {
    if (
        (mode == Mode::Machine && ((load_csr(MSTATUS) >> 3) & 1) == 0) ||
        (mode == Mode::Supervisor && ((load_csr(SSTATUS) >> 1) & 1) == 0)) {
        return std::nullopt;
    }

    uint64_t irq;
    if (bus.uart.is_interrupting()) {
        irq = UART_IRQ;
    } else if (bus.virtio.is_interrupting()) {
        disk_access();
        irq = VIRTIO_IRQ;
    } else {
        irq = 0;
    }

    if (irq != 0) {
        store(PLIC_SCLAIM, 4, irq);
        store_csr(MIP, load_csr(MIP) | MIP_SEIP);
    }

    auto pending = load_csr(MIE) & load_csr(MIP);

    if ((pending & MIP_MEIP) != 0) {
        store_csr(MIP, load_csr(MIP) & ~MIP_MEIP);
        return Interrupt(InterruptType::MachineExternalInterrupt);
    }
    if ((pending & MIP_MSIP) != 0) {
        store_csr(MIP, load_csr(MIP) & ~MIP_MSIP);
        return Interrupt(InterruptType::MachineSoftwareInterrupt);
    }
    if ((pending & MIP_MTIP) != 0) {
        store_csr(MIP, load_csr(MIP) & ~MIP_MTIP);
        return Interrupt(InterruptType::MachineTimerInterrupt);
    }
    if ((pending & MIP_SEIP) != 0) {
        store_csr(MIP, load_csr(MIP) & ~MIP_SEIP);
        return Interrupt(InterruptType::SupervisorExternalInterrupt);
    }
    if ((pending & MIP_SSIP) != 0) {
        store_csr(MIP, load_csr(MIP) & ~MIP_SSIP);
        return Interrupt(InterruptType::SupervisorSoftwareInterrupt);
    }
    if ((pending & MIP_STIP) != 0) {
        store_csr(MIP, load_csr(MIP) & ~MIP_STIP);
        return Interrupt(InterruptType::SupervisorTimerInterrupt);
    }
    return std::nullopt;
}

void Cpu::disk_access() {
    auto desc_addr = bus.virtio.desc_addr();
    auto avail_addr = bus.virtio.desc_addr() + 0x40;
    auto used_addr = bus.virtio.desc_addr() + 4096;

    auto [offset, offset_err] = load(avail_addr + 1, 2);
    if (offset_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto [index, index_err] = load(avail_addr + (offset % DESC_NUM) + 2, 2);
    if (index_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto desc_addr0 = desc_addr + VRING_DESC_SIZE * index;
    auto [addr0, addr0_err] = load(desc_addr0, 8);
    if (addr0_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto [next0, next_err] = load(desc_addr0 + 14, 2);
    if (next_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto desc_addr1 = desc_addr + VRING_DESC_SIZE * next0;

    auto [addr1, addr1_err] = load(desc_addr1, 8);
    if (addr1_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto [len1, len1_err] = load(desc_addr1 + 8, 4);
    if (len1_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto [flags1, flags1_err] = load(desc_addr1 + 12, 2);
    if (flags1_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto [blk_sector, blk_sector_err] = load(addr0 + 8, 8);
    if (blk_sector_err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if ((flags1 & 2) == 0) {
        for (uint64_t i = 0; i < len1; i++) {
            auto [data, data_err] = load(addr1 + i, 1);
            if (data_err.has_value()) {
                std::cerr << "error: disk_access()" << std::endl;
                std::exit(EXIT_FAILURE);
            }
            bus.virtio.write_disk(blk_sector * 512 + i, data);
        }
    } else {
        for (uint64_t i = 0; i < len1; i++) {
            auto data = bus.virtio.read_disk(blk_sector * 512 + i);
            auto data_err = store(addr1 + i, 1, data);
            if (data_err.has_value()) {
                std::cerr << "error: disk_access()" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }

    auto new_id = bus.virtio.get_new_id();
    auto err = store(used_addr + 2, 2, new_id % 8);
    if (err.has_value()) {
        std::cerr << "error: disk_access()" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Cpu::update_paging(uint64_t csr_addr) {
    if (csr_addr != SATP) {
        return;
    }

    page_table = (load_csr(SATP) & (((uint64_t)1 << 44) - 1)) * PAGE_SIZE;

    if ((load_csr(SATP) >> 60) == 8) {
        enable_paging = true;
    } else {
        enable_paging = false;
    }
}

std::pair<uint64_t, std::optional<Exception>> Cpu::translate(uint64_t addr, AccessType access_type) {
    if (!enable_paging) {
        return std::make_pair(addr, std::nullopt);
    }

    auto levels = 3;
    uint64_t vpn[] = {
        (addr >> 12) & 0x1ff,
        (addr >> 21) & 0x1ff,
        (addr >> 30) & 0x1ff,
    };

    auto a = page_table;
    auto i = levels - 1;
    uint64_t pte;

    while (true) {
        auto [data, err] = bus.load(a + vpn[i] * 8, 8);
        if (err.has_value()) {
            std::cerr << "error: translate()" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        pte = data;

        auto v = pte & 1;
        auto r = (pte >> 1) & 1;
        auto w = (pte >> 2) & 1;
        auto x = (pte >> 3) & 1;
        if (v == 0 || (r == 0 && w == 1)) {
            switch (access_type) {
            case AccessType::Instruction:
                return std::make_pair(0, Exception(ExceptionType::InstructionPageFault));
            case AccessType::Load:
                return std::make_pair(0, Exception(ExceptionType::LoadPageFault));
            case AccessType::Store:
                return std::make_pair(0, Exception(ExceptionType::StoreAMOPageFault));
            default:
                std::cerr << "error: illegal AccessType" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }

        if (r == 1 || x == 1) {
            break;
        }
        i--;
        auto ppn = (pte >> 10) & 0x0fff'ffff'ffff;
        a = ppn * PAGE_SIZE;
        if (i < 0) {
            switch (access_type) {
            case AccessType::Instruction:
                return std::make_pair(0, Exception(ExceptionType::InstructionPageFault));
            case AccessType::Load:
                return std::make_pair(0, Exception(ExceptionType::LoadPageFault));
            case AccessType::Store:
                return std::make_pair(0, Exception(ExceptionType::StoreAMOPageFault));
            default:
                std::cerr << "error: illegal AccessType" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }

    auto offset = addr & 0xfff;

    switch (i) {
    case 0: {
        auto ppn = (pte >> 10) & 0x0fff'ffff'ffff;
        return std::make_pair((ppn << 12) | offset, std::nullopt);
    }
    case 1:
    case 2: {
        uint64_t ppn[] = {
            (pte >> 10) & 0x1ff,
            (pte >> 19) & 0x1ff,
            (pte >> 28) & 0x03ffffff,
        };
        return std::make_pair((ppn[2] << 30) | (ppn[1] << 21) | (vpn[0] << 12) | offset, std::nullopt);
    }
    default:
        switch (access_type) {
        case AccessType::Instruction:
            return std::make_pair(0, Exception(ExceptionType::InstructionPageFault));
        case AccessType::Load:
            return std::make_pair(0, Exception(ExceptionType::LoadPageFault));
        case AccessType::Store:
            return std::make_pair(0, Exception(ExceptionType::StoreAMOPageFault));
        default:
            std::cerr << "error: illegal AccessType" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
}
