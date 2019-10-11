#pragma once

#include <cstdint>

#define YB_MEM_SIZE (0xFFFF)

namespace yb {

    class MMU {
    public:
        MMU(uint8_t* cartridge);

        uint8_t read8(uint16_t addr) const;

    private:
        uint8_t* cartridge_;
        uint8_t ram_[YB_MEM_SIZE];
    };

}