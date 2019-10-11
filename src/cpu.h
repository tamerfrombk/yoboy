#pragma once

#include <cstdint>
#include "mmu.h"

namespace yb {

    union Register
    {
        uint16_t value;
        struct
        {
            // TODO: this only works on little endian systems where we read the words LSB first
            // Fix this to work for big endian systems where we flip the word order
            uint8_t lo;
            uint8_t hi;
        };
    };

    struct CPU
    {
        CPU(yb::MMU* mmu);

        uint8_t cycle();
    
        Register AF;
        Register BC;
        Register DE;
        Register HL;

        Register SP;
        Register PC;

        yb::MMU* mmu_;

    };
}
