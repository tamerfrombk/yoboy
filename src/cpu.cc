#include "cpu.h"

#include <cstdio>

#include "ops.h"
#include "common.h"

namespace yb {

static constexpr uint8_t ZF = (1 << 7);
static constexpr uint8_t NF = (1 << 6);
static constexpr uint8_t HF = (1 << 5);
static constexpr uint8_t CF = (1 << 4);

static void jump_if(yb::CPU* cpu, yb::MMU* mmu, bool cond)
{
    if (cond) {
        const uint16_t target = mmu->read16(cpu->PC.value + 1);
        yb::log("JP target: 0x%.4X.\n", target);
        cpu->PC.value = target;
    }
}

// Apparently, 'xor' is a keyword in C++. Who knew?
static void xor_(yb::CPU* cpu, uint8_t n)
{
    cpu->AF.hi |= n;
    if (cpu->AF.hi == 0) {
        cpu->AF.lo |= ZF;
    }
    cpu->AF.lo &= ~(NF | HF | CF);
}

}

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
   uint8_t op = mmu_->read8(PC.value);
   yb::log("Fetching from 0x%.4X: 0x%.2X.\n", PC.value, op);

   // decode
   const yb::Instruction& inst = yb::INSTRUCTIONS.at(op);

   // execute
   switch (op) {
    case 0x00:
        PC.value += inst.length;
        return inst.cycles;
    case 0xC3: 
        jump_if(this, mmu_, true);
        return inst.cycles;
    case 0xAF:
        xor_(this, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x21:
        HL.value = mmu_->read16(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x0E:
        BC.lo = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x06:
        BC.hi = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    default:
        yb::exit("Unknown instruction 0x%.2X.\n", op);
        return 0;
   }
}