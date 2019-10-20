#include "emulator.h"
#include <cstdio>

yb::Emulator::Emulator(yb::Cartridge cartridge)
    : cartridge_(std::move(cartridge))
    , mmu_(cartridge_.data())
    , cpu_(&mmu_)
    , window_("yoboy", 160, 144)
{}
        
bool yb::Emulator::isRunning() const
{
    return !window_.isQuit();
}

void yb::Emulator::start()
{
    std::puts("Emulation started.");
    while (isRunning()) {
        cpu_.tick();
        window_.update();
        window_.draw();
    }
}
