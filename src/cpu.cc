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
   yb::log("Fetching from 0x%.4X.\n", PC.value);
   uint8_t op = mmu_->read8(PC.value);

   // decode
   const yb::Instruction& inst = yb::INSTRUCTIONS.at(op);

   // execute
   switch (op) {
    case 0x00:
        PC.value += inst.length;
        return inst.cycles;
    case 0xC3: {
        const uint16_t target = mmu_->read16(PC.value + 1);
        yb::log("JP target: 0x%.4X.\n", target);
        PC.value = target;
        return inst.cycles;
    }
    default:
        yb::exit("Unknown instruction 0x%.2X.\n", op);
        return 0;
   }
}
