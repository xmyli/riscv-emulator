#include "uart.h"

#include <iostream>
#include <thread>
#include <string>

Uart::Uart() : lock{std::mutex()},
               condvar{std::condition_variable()},
               buffer{std::vector<uint8_t>(UART_SIZE, 0)},
               interrupting{false}
{
    buffer[UART_LSR - UART_BASE] |= UART_LSR_TX;
    auto reader = std::thread(&Uart::listen, this);
    reader.detach();
}

void Uart::listen()
{
    char c;
    while (std::cin >> c)
    {
        std::unique_lock<std::mutex> ulock(lock);
        while ((buffer[UART_LSR - UART_BASE] & UART_LSR_RX) == 1)
        {
            condvar.wait(ulock);
        }
        if (c == '_')
        {
            buffer[UART_RHR - UART_BASE] = ' ';
        }
        else if (c == ';')
        {
            buffer[UART_RHR - UART_BASE] = '\n';
        }
        else
        {
            buffer[UART_RHR - UART_BASE] = c;
        }
        interrupting.store(true);
        buffer[UART_LSR - UART_BASE] |= UART_LSR_RX;
    }
}

std::pair<uint64_t, std::optional<Exception>> Uart::load(uint64_t addr, int nBytes)
{
    if (nBytes == 1)
    {
        std::lock_guard<std::mutex> guard(lock);
        if (addr == UART_RHR)
        {
            condvar.notify_one();
            buffer[UART_LSR - UART_BASE] &= ~UART_LSR_RX;
            return std::make_pair(buffer[UART_RHR - UART_BASE], std::nullopt);
        }
        return std::make_pair(buffer[addr - UART_BASE], std::nullopt);
    }
    return std::make_pair(0, Exception(ExceptionType::LoadAccessFault));
}

std::optional<Exception> Uart::store(uint64_t addr, int nBytes, uint64_t value)
{
    if (nBytes == 1)
    {
        std::lock_guard<std::mutex> guard(lock);
        if (addr == UART_THR)
        {
            std::cout << (char)value << std::flush;
            return std::nullopt;
        }
        buffer[addr - UART_BASE] = value;
        return std::nullopt;
    }
    return Exception(ExceptionType::StoreAMOAccessFault);
}

bool Uart::is_interrupting()
{
    return interrupting.exchange(false, std::memory_order_acquire);
}