#pragma once

#include <vector>
#include <cstdint>

namespace yb {

    struct Cartridge
    {
    public:
        Cartridge(std::vector<std::uint8_t> mem);
        
        bool empty() const;

        uint8_t* data();

    private:
        std::vector<std::uint8_t> mem_;
    };
}
