#include "emulator.h"
#include <cstdio>

yb::Emulator::Emulator(yb::Cartridge cartridge)
    : cartridge_(std::move(cartridge))
    {}
        
bool yb::Emulator::isRunning() const
{
    return true;
}

void yb::Emulator::start()
{
    std::puts("Emulation started.");
    while (isRunning()) {
        ;
    }
}