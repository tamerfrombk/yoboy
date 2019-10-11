#include "cartridge.h"

yb::Cartridge::Cartridge(std::vector<std::uint8_t> mem)
    : mem_(std::move(mem))
{}

bool yb::Cartridge::empty() const
{
    return mem_.empty();
}

uint8_t* yb::Cartridge::data()
{
    return mem_.data();
}
