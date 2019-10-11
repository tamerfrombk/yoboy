#pragma once

#include <vector>
#include <cstdint>

namespace yb {

    // TODO: add other MCB types
    enum class CartridgeType
    {
        ROM_ONLY = 0,
        ROM_MBC1
    };

    struct Cartridge
    {
    public:
        Cartridge(std::vector<std::uint8_t> mem);
        
        bool empty() const;

        uint8_t* data();

        CartridgeType type() const;

    private:
        std::vector<std::uint8_t> mem_;
        CartridgeType type_;
    };
}
