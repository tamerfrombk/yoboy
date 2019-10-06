#pragma once

#include "cartridge.h"

namespace yb {

    class Emulator
    {
    public:
        Emulator(yb::Cartridge cartridge);
        
        bool isRunning() const;

        void start();

    private:
        yb::Cartridge cartridge_;
    };
}