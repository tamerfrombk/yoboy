#pragma once

#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"

namespace yb {

    class Emulator
    {
    public:
        Emulator(yb::Cartridge cartridge);
        
        bool isRunning() const;

        void start();

    private:
        yb::Cartridge cartridge_;
        yb::MMU mmu_;
        yb::CPU cpu_;
    };
}
