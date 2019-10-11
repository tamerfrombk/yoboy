#include "cartridge.h"

#include <cstdio>
#include <cstring>

#include "common.h"

yb::Cartridge::Cartridge(std::vector<std::uint8_t> mem)
    : mem_(std::move(mem))
{
    {
        char title[16 + 1] = {0};
        std::memcpy(title, (char*) mem_.data() + 0x134, 16);
        std::printf("Title: %s\n", title);
    }

    type_ = (yb::CartridgeType) mem_[0x147];

    // TODO: support other cartridge types
    if (type_ != yb::CartridgeType::ROM_ONLY) {
        yb::exit("ROM_ONLY cartridge is currently supported.\n");
    }

    std::printf("Catridge Type: %d\n", (int) type_);
}

bool yb::Cartridge::empty() const
{
    return mem_.empty();
}

uint8_t* yb::Cartridge::data()
{
    return mem_.data();
}

yb::CartridgeType yb::Cartridge::type() const
{
    return type_;
}
