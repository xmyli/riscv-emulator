#ifndef UART_H
#define UART_H

#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "device.h"

#define UART_BASE 0x10000000
#define UART_SIZE 0x100
#define UART_IRQ ((uint64_t)10)
#define UART_RHR (UART_BASE + 0)
#define UART_THR (UART_BASE + 0)
#define UART_LCR (UART_BASE + 3)
#define UART_LSR (UART_BASE + 5)
#define UART_LSR_RX 1
#define UART_LSR_TX (1 << 5)

class Uart : public Device
{
private:
    std::mutex lock;
    std::condition_variable condvar;
    std::vector<uint8_t> buffer;
    std::atomic<bool> interrupting;

public:
    Uart();
    std::pair<uint64_t, std::optional<Exception>> load(uint64_t addr, int nBytes);
    std::optional<Exception> store(uint64_t addr, int nBytes, uint64_t value);
    void listen();
    bool is_interrupting();
};

#endif
