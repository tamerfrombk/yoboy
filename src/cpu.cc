#include "cpu.h"

#include <cstdio>

yb::CPU::CPU()
{
    AF.value = 0xABCD;

    std::printf("hi: 0x%.2X lo: 0x%.2X   value: 0x%.4x\n", AF.hi, AF.lo, AF.value);
}
