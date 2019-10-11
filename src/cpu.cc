#include "cpu.h"

#include <cstdio>

#include "ops.h"
#include "common.h"

yb::CPU::CPU(yb::MMU* mmu)
    : mmu_(mmu)
{
    AF.value = 0x01B0;
    BC.value = 0x0013;
    DE.value = 0x00D8;
    HL.value = 0x014D;
    SP.value = 0xFFFE;

    // TODO: verify this value
    PC.value = 0x100;
}

uint8_t yb::CPU::cycle()
{
    // fetch
   std::printf("Fetching from 0x%.4X.\n", PC.value);
   const uint8_t op = mmu_->read8(PC.value);

   // decode
   yb::Instruction inst = yb::INSTRUCTIONS.at(op);

   // execute
   switch (op) {
    default:
        yb::exit("Unknown instruction 0x%.2X.\n", op);
        return 0;
   }
}
