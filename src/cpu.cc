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

static uint8_t dec8(yb::CPU* cpu, uint8_t n)
{
    const uint8_t result = n - 1;
    if (result == 0) {
        cpu->AF.lo |= ZF;
    }

    cpu->AF.lo |= NF;

    // Assuming A = LHS and B = RHS ...
    // We have to borrow from bit 4 if bit 3 is not set in A but it is set in B
    // 00000100 = 4
    // 00001000 = 8
    //     ^--------- Bit 3 is not set in A but set in B
    // --
    // 11111100 = -4
    //
    // However, given that B will always be 1 in this case, B's bit 3 will always not be set.
    // 00000001 = 1
    //     ^--------- Bit 3 is not set
    //
    // This means the carry will happen only if the most significant set bit is bit 4. 
    // In other words, if none of the lower 4 bits are set, then the carry will happen.
    // 00010100 = 20
    // 00000001 = 1
    // --
    // 00010011 = 19
    //    ^---------- No carry happened at bit 4
    //
    // Set H if no borrow from bit 4 happens
    if (!(n & 0xF)) {
        cpu->AF.lo |= HF;
    }

    return result;
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
    case 0x32:
        mmu_->write8(HL.value, AF.lo);
        HL.value -= 1;
        PC.value += inst.length;
        return inst.cycles;
    // DEC n
    case 0x3D:
        AF.hi = dec8(this, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x05:
        BC.hi = dec8(this, BC.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x0D:
        BC.lo = dec8(this, BC.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0x15:
        DE.hi = dec8(this, DE.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x1D:
        DE.lo = dec8(this, DE.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0x25:
        HL.hi = dec8(this, HL.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x2D:
        HL.lo = dec8(this, HL.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0x35: {
        const uint8_t value = dec8(this, mmu_->read8(HL.value));
        mmu_->write8(HL.value, value);
        PC.value += inst.length;
        return inst.cycles;
    }
    // JR (cc), n
    case 0x20: {
        if ((AF.lo & ZF) == 0) {
            const uint16_t target = (int8_t)mmu_->read8(PC.value + 1) + PC.value + inst.length;
            yb::log("JP target: 0x%.4X.\n", target);
            PC.value = target;
        } else {
            PC.value += inst.length;
        }
        return inst.cycles;
    }
    case 0x28: {
        if (AF.lo & ZF) {
            const uint16_t target = (int8_t)mmu_->read8(PC.value + 1) + PC.value + inst.length;
            yb::log("JP target: 0x%.4X.\n", target);
            PC.value = target;
        } else {
            PC.value += inst.length;
        }
        return inst.cycles;
    }
    case 0x30: {
        if ((AF.lo & CF) == 0) {
            const uint16_t target = (int8_t)mmu_->read8(PC.value + 1) + PC.value + inst.length;
            yb::log("JP target: 0x%.4X.\n", target);
            PC.value = target;
        } else {
            PC.value += inst.length;
        }
        return inst.cycles;
    }
    case 0x38: {
        if (AF.lo & CF) {
            const uint16_t target = (int8_t)mmu_->read8(PC.value + 1) + PC.value + inst.length;
            yb::log("JP target: 0x%.4X.\n", target);
            PC.value = target;
        } else {
            PC.value += inst.length;
        }
        return inst.cycles;
    }
    default:
        yb::exit("Unknown instruction 0x%.2X.\n", op);
        return 0;
   }
}
