#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <iterator>

#include "cpu.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "error: invalid number of parameters" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> binary;
    std::vector<uint8_t> disk_image;

    {
        std::ifstream file(argv[1], std::ifstream::binary);
        file.seekg(0, file.end);
        auto binary_size = file.tellg();
        file.seekg(0, file.beg);
        binary.resize(binary_size / sizeof(uint8_t));
        file.read(reinterpret_cast<char *>(binary.data()), binary.size() * sizeof(uint8_t));
        file.close();
    }

    {
        std::ifstream file(argv[2], std::ifstream::binary);
        file.seekg(0, file.end);
        auto binary_size = file.tellg();
        file.seekg(0, file.beg);
        disk_image.resize(binary_size / sizeof(uint8_t));
        file.read(reinterpret_cast<char *>(disk_image.data()), disk_image.size() * sizeof(uint8_t));
        file.close();
    }

    Cpu cpu(binary, disk_image);

    while (true)
    {
        auto [instruction, f_err] = cpu.fetch();
        if (f_err.has_value())
        {
            cpu.take_trap(f_err.value(), false);
            if (f_err->is_fatal())
            {
                break;
            }
        }

        cpu.setPc(cpu.getPc() + 4);

        auto e_err = cpu.execute(instruction);
        if (e_err.has_value())
        {
            cpu.take_trap(e_err.value(), false);
            if (e_err->is_fatal())
            {
                break;
            }
        }

        auto interrupt = cpu.check_pending_interrupt();
        if (interrupt.has_value())
        {
            cpu.take_trap(interrupt.value(), true);
        }
    }

    return EXIT_SUCCESS;
}
