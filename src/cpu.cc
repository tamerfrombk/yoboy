#include "cpu.h"

#include <cstdio>

#include "ops.h"
#include "common.h"

namespace yb {

static constexpr uint8_t ZF = (1 << 7);
static constexpr uint8_t NF = (1 << 6);
static constexpr uint8_t HF = (1 << 5);
static constexpr uint8_t CF = (1 << 4);

// Assuming a = LHS, b = RHS, and r is the result...
// We will always have to borrow from bit 4 (half carry)
// if A is smaller than B in the bottom 4 bits.
//
// For example, this subtraction leads to a half borrow:
// a) 00000100 = 4
// b) 00001000 = 8
// --
// r) 11111100 = -4
// Another example where a borrow is required but the numbers 
// aren't already smaller than what can fit in 4 bits:
// a) 00010000 = 16
// b) 00100001 = 33
// --
// r) 11101111 = -17
//
// The below equation will then compare 0 (bottom 4 bits of a)
// against 1 (bottom 4 bits of b) to determine if a half carry
// is necessary.
static bool half_borrow(uint8_t a, uint8_t b)
{
    return (a & 0xF) < (b & 0xF);
}

// Assuming a = LHS, b = RHS, and r is the result...
// We will always have to carry from bit 3
// if the result is larger that 15 when taking into account the lower nibble.
// For example, here a carry is required:
// a) 00001000 = 8
// b) 00001000 = 8
// --
// r) 00010000 = 16
// where we don't require one here:
// a) 00000001 = 1
// b) 00000010 = 2
// --
// r) 00000011 = 3
// or here: 
// a) 00001000 = 8
// b) 00000001 = 1
// --
// r) 00001001 = 9
static bool half_carry(uint8_t a, uint8_t b)
{
    return ((a & 0xF) + (b & 0xF)) > 0xF;
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

    if (!half_borrow(n, 1)) {
        cpu->AF.lo |= HF;
    }

    return result;
}

static uint8_t inc8(yb::CPU* cpu, uint8_t n)
{
    const uint8_t result = n + 1;
    if (result == 0) {
        cpu->AF.lo |= ZF;
    }

    cpu->AF.lo &= ~(NF);

    if (half_carry(n, 1)) {
        cpu->AF.lo |= HF;
    }

    return result;
}

static void cp(yb::CPU* cpu, uint8_t n)
{
    const uint8_t result = cpu->AF.hi - n;
    if (result == 0) {
        cpu->AF.lo |= ZF;
    }

    cpu->AF.lo |= NF;

    if (!half_borrow(cpu->AF.hi, n)) {
        cpu->AF.lo |= HF;
    }

    if (cpu->AF.hi < n) {
        cpu->AF.lo |= CF;
    }
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
    // LD nn, n
    case 0x06:
        BC.hi = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x0E:
        BC.lo = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x16:
        DE.hi = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x1E:
        DE.lo = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x26:
        HL.hi = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x2E:
        HL.lo = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (A)
    case 0x7F:
        // This instruction technically assigns A to itself but it's been implemented
        // as a nop.
        PC.value += inst.length;
        return inst.cycles;
    case 0x78:
        AF.hi = BC.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x79:
        AF.hi = BC.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x7A:
        AF.hi = DE.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x7B:
        AF.hi = DE.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x7C:
        AF.hi = HL.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x7D:
        AF.hi = HL.lo;
        PC.value += inst.length;
        return inst.cycles;    
    case 0x7E:
        AF.hi = mmu_->read8(HL.value);
        PC.value += inst.length;
        return inst.cycles;
    case 0x0A:
        AF.hi = mmu_->read8(BC.value);
        PC.value += inst.length;
        return inst.cycles;
    case 0x1A:
        AF.hi = mmu_->read8(DE.value);
        PC.value += inst.length;
        return inst.cycles;
    case 0xFA: {
        const uint16_t nn = mmu_->read16(PC.value + 1);
        AF.hi = mmu_->read8(nn);
        PC.value += inst.length;
        return inst.cycles;
    }
    case 0x3E:
        AF.hi = mmu_->read8(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (B)
    case 0x40:
        // This instruction technically assigns B to itself but it's been implemented
        // as a nop.
        PC.value += inst.length;
        return inst.cycles;
    case 0x41:
        BC.hi = BC.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x42:
        BC.hi = DE.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x43:
        BC.hi = DE.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x44:
        BC.hi = HL.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x45:
        BC.hi = HL.lo;
        PC.value += inst.length;
        return inst.cycles;    
    case 0x46:
        BC.hi = mmu_->read8(HL.value);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (C)
    case 0x48:
        BC.lo = BC.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x49:
        // This instruction technically assigns C to itself but it's been implemented
        // as a nop
        PC.value += inst.length;
        return inst.cycles;
    case 0x4A:
        BC.lo = DE.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x4B:
        BC.lo = DE.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x4C:
        BC.lo = HL.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x4D:
        BC.lo = HL.lo;
        PC.value += inst.length;
        return inst.cycles;    
    case 0x4E:
        BC.lo = mmu_->read8(HL.value);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (D)
    case 0x50:
        DE.hi = BC.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x51:
        DE.hi = BC.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x52:
        // This instruction technically assigns D to itself but it's been implemented
        // as a nop
        PC.value += inst.length;
        return inst.cycles;
    case 0x53:
        DE.hi = DE.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x54:
        DE.hi = HL.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x55:
        DE.hi = HL.lo;
        PC.value += inst.length;
        return inst.cycles;    
    case 0x56:
        DE.hi = mmu_->read8(HL.value);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (E)
    case 0x58:
        DE.lo = BC.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x59:
        DE.lo = BC.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x5A:
        DE.lo = DE.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x5B:
        // This instruction technically assigns E to itself but it's been implemented
        // as a nop
        PC.value += inst.length;
        return inst.cycles;
    case 0x5C:
        DE.lo = HL.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x5D:
        DE.lo = HL.lo;
        PC.value += inst.length;
        return inst.cycles;    
    case 0x5E:
        DE.lo = mmu_->read8(HL.value);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (H)
    case 0x60:
        HL.hi = BC.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x61:
        HL.hi = BC.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x62:
        HL.hi = DE.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x63:
        HL.hi = DE.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x64:
        // This instruction technically assigns H to itself but it's been implemented
        // as a nop
        PC.value += inst.length;
        return inst.cycles;
    case 0x65:
        HL.hi = HL.lo;
        PC.value += inst.length;
        return inst.cycles;    
    case 0x66:
        HL.hi = mmu_->read8(HL.value);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (L)
    case 0x68:
        HL.lo = BC.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x69:
        HL.lo = BC.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x6A:
        HL.lo = DE.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x6B:
        HL.lo = DE.lo;
        PC.value += inst.length;
        return inst.cycles;
    case 0x6C:
        HL.lo = HL.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x6D:
        // This instruction technically assigns L to itself but it's been implemented
        // as a nop
        PC.value += inst.length;
        return inst.cycles;    
    case 0x6E:
        HL.lo = mmu_->read8(HL.value);
        PC.value += inst.length;
        return inst.cycles;
    // LD r1, r2 (HL)
    case 0x70:
        mmu_->write8(HL.value, BC.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x71:
        mmu_->write8(HL.value, BC.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0x72:
        mmu_->write8(HL.value, DE.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x73:
        mmu_->write8(HL.value, DE.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0x74:
        mmu_->write8(HL.value, HL.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x75:
        mmu_->write8(HL.value, HL.lo);
        PC.value += inst.length;
        return inst.cycles;    
    case 0x36: {
        const uint8_t n = mmu_->read8(PC.value + 1);
        mmu_->write8(HL.value, n);
        PC.value += inst.length;
        return inst.cycles;
    }
    case 0x47:
        BC.hi = AF.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x4F:
        BC.lo = AF.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x57:
        DE.hi = AF.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x5F:
        DE.lo = AF.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x67:
        HL.hi = AF.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x6F:
        HL.lo = AF.hi;
        PC.value += inst.length;
        return inst.cycles;
    case 0x02:
        mmu_->write8(BC.value, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x12:
        mmu_->write8(DE.value, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x77:
        mmu_->write8(HL.value, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0xEA:
        mmu_->write8(mmu_->read16(PC.value + 1), AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    // LD n, nn
    case 0x01:
        BC.value = mmu_->read16(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x11:
        DE.value = mmu_->read16(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x21:
        HL.value = mmu_->read16(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    case 0x31:
        SP.value = mmu_->read16(PC.value + 1);
        PC.value += inst.length;
        return inst.cycles;
    // LD SP, HL
    case 0xF9:
        SP.value = HL.value;
        PC.value += inst.length;
        return inst.cycles;
    // LDI A, (HL)
    case 0x2A:
        AF.hi = mmu_->read8(HL.value);
        HL.value += 1;
        PC.value += inst.length;
        return inst.cycles;
    // LD (C), A
    case 0xE2:
        mmu_->write8(0xFF00 + BC.lo, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    // INC n
    case 0x3C:
        AF.hi = inc8(this, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x04:
        BC.hi = inc8(this, BC.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x0C:
        BC.lo = inc8(this, BC.lo);    
        PC.value += inst.length;
        return inst.cycles;
    case 0x14:
        DE.hi = inc8(this, DE.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x1C:
        DE.lo = inc8(this, DE.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0x24:
        HL.hi = inc8(this, HL.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0x2C:
        HL.lo = inc8(this, HL.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0x34: {
        const uint8_t value = mmu_->read8(HL.value);
        mmu_->write8(HL.value, inc8(this, value));
        PC.value += inst.length;
        return inst.cycles;
    }
    // CALL nn
    case 0xCD: {
        SP.value = PC.value + inst.length;
        const uint16_t target = mmu_->read16(PC.value + 1);
        yb::log("CALL target: 0x%.4X.\n", target);
        PC.value = target;
        return inst.cycles;
    }
    // DEC nn
    case 0x0B:
        BC.value -= 1;
        PC.value += inst.length;
        return inst.cycles;
    case 0x1B:
        DE.value -= 1;
        PC.value += inst.length;
        return inst.cycles;
    case 0x2B:
        HL.value -= 1;
        PC.value += inst.length;
        return inst.cycles;
    case 0x3B:
        SP.value -= 1;
        PC.value += inst.length;
        return inst.cycles;
    // NOP
    case 0x00:
        PC.value += inst.length;
        return inst.cycles;
    case 0xC3: {
        const uint16_t target = mmu_->read16(PC.value + 1);
        yb::log("JP target: 0x%.4X.\n", target);
        PC.value = target;
        return inst.cycles; 
    }
    case 0xAF:
        xor_(this, AF.hi);
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
    // DI
    case 0xF3:
        // TODO: disable interrupt
        PC.value += inst.length;
        return inst.cycles;
    // LDH (n), A
    case 0xE0: {
        const uint8_t n = mmu_->read8(PC.value + 1);
        mmu_->write8(0xFF00 + n, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    }
    // LDH A,n
    case 0xF0: {
        const uint8_t n = mmu_->read8(PC.value + 1);
        AF.hi = mmu_->read8(0xFF00 + n);
        PC.value += inst.length;
        return inst.cycles;
    }
    // CP n
    case 0xBF:
        cp(this, AF.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0xB8:
        cp(this, BC.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0xB9:
        cp(this, BC.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0xBA:
        cp(this, DE.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0xBB:
        cp(this, DE.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0xBC:
        cp(this, HL.hi);
        PC.value += inst.length;
        return inst.cycles;
    case 0xBD:
        cp(this, HL.lo);
        PC.value += inst.length;
        return inst.cycles;
    case 0xBE:
        cp(this, mmu_->read8(HL.value));
        PC.value += inst.length;
        return inst.cycles;
    case 0xFE:
        cp(this, mmu_->read8(PC.value + 1));
        PC.value += inst.length;
        return inst.cycles;
    default:
        yb::exit("Unknown instruction 0x%.2X.\n", op);
        return 0;
   }
}
