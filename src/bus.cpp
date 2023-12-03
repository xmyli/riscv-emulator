#include <iostream>

#include "bus.h"

Bus::Bus(std::vector<uint8_t> bytes, std::vector<uint8_t> disk_image) : memory{Memory(bytes)},
                                                                        clint{CLINT()},
                                                                        plic{PLIC()},
                                                                        uart{Uart()},
                                                                        virtio{Virtio(disk_image)}
{
}

std::pair<uint64_t, std::optional<Exception>> Bus::load(uint64_t addr, int N)
{
    if (CLINT_BASE <= addr && addr < CLINT_BASE + CLINT_SIZE)
    {
        auto [data, err] = clint.load(addr, N);
        if (err.has_value())
        {
            return std::make_pair(0, err);
        }
        return std::make_pair(data, std::nullopt);
    }
    if (PLIC_BASE <= addr && addr < PLIC_BASE + PLIC_SIZE)
    {
        auto [data, err] = plic.load(addr, N);
        if (err.has_value())
        {
            return std::make_pair(0, err);
        }
        return std::make_pair(data, std::nullopt);
    }
    if (UART_BASE <= addr && addr < UART_BASE + UART_SIZE)
    {
        auto [data, err] = uart.load(addr, N);
        if (err.has_value())
        {
            return std::make_pair(0, err);
        }
        return std::make_pair(data, std::nullopt);
    }
    if (VIRTIO_BASE <= addr && addr < VIRTIO_BASE + VIRTIO_SIZE)
    {
        auto [data, err] = virtio.load(addr, N);
        if (err.has_value())
        {
            return std::make_pair(0, err);
        }
        return std::make_pair(data, std::nullopt);
    }
    if (MEMORY_BASE <= addr)
    {
        auto [data, err] = memory.load(addr, N);
        if (err.has_value())
        {
            return std::make_pair(0, err);
        }
        return std::make_pair(data, std::nullopt);
    }
    return std::make_pair(0, Exception(ExceptionType::LoadAccessFault));
}

std::optional<Exception> Bus::store(uint64_t addr, int N, uint64_t value)
{
    if (CLINT_BASE <= addr && addr < CLINT_BASE + CLINT_SIZE)
    {
        auto err = clint.store(addr, N, value);
        if (err.has_value())
        {
            return err;
        }
        return std::nullopt;
    }
    if (PLIC_BASE <= addr && addr < PLIC_BASE + PLIC_SIZE)
    {
        auto err = plic.store(addr, N, value);
        if (err.has_value())
        {
            return err;
        }
        return std::nullopt;
    }
    if (UART_BASE <= addr && addr < UART_BASE + UART_SIZE)
    {
        auto err = uart.store(addr, N, value);
        if (err.has_value())
        {
            return err;
        }
        return std::nullopt;
    }
    if (VIRTIO_BASE <= addr && addr < VIRTIO_BASE + VIRTIO_SIZE)
    {
        auto err = virtio.store(addr, N, value);
        if (err.has_value())
        {
            return err;
        }
        return std::nullopt;
    }
    if (MEMORY_BASE <= addr)
    {
        auto err = memory.store(addr, N, value);
        if (err.has_value())
        {
            return err;
        }
        return std::nullopt;
    }
    return Exception(ExceptionType::StoreAMOAccessFault);
}
