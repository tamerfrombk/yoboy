#pragma once

#include <vector>
#include <cstdint>

namespace yb {

    struct Cartridge
    {
        Cartridge(std::vector<std::uint8_t> mem)
            : mem_(std::move(mem))
            {}
        
        inline bool empty() const
        {
            return mem_.empty();
        }
        
        std::vector<std::uint8_t> mem_;
    };
}