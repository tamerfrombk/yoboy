#include <cstring>

#include "mmu.h"

yb::MMU::MMU(uint8_t* cartridge)
    : cartridge_(cartridge)
{
    std::memset(ram_, 0, sizeof(uint8_t) * YB_MEM_SIZE);
    
    // TODO: for now, assume MCB zero and copy entirety of game into first two ROM banks
    std::memcpy(ram_, cartridge_, 0x8000 * sizeof(uint8_t));
}

uint8_t yb::MMU::read8(uint16_t addr) const
{
    // TODO: add checks
    return ram_[addr];
}
