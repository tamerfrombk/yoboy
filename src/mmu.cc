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

uint16_t yb::MMU::read16(uint16_t addr) const
{
    // TODO: add checks
    const uint16_t value = (uint16_t)ram_[addr + 1] << 8 | ram_[addr];
    return value;
}

void yb::MMU::write8(uint16_t addr, uint8_t value)
{
    // TODO: add checks
    ram_[addr] = value;
}

void yb::MMU::write16(uint16_t addr, uint16_t value)
{
    // TODO: add checks
    ram_[addr] = value >> 8;
    ram_[addr + 1] = value & 0xFF;
}