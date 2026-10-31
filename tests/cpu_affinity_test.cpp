// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/android_performance.h"
#include <cassert>
#include <iostream>
using namespace hotd_model2;
int main(){
    std::array<uint64_t,64> frequency{};
    frequency[0]=frequency[1]=2054400;
    for(unsigned i=2;i<6;++i)frequency[i]=2361600;
    assert(fast_cpu_mask(0x3f,frequency)==0x3c); // Measured Quest3S clusters.
    assert(fast_cpu_mask(0x0f,frequency)==0x0c); // Respect the app's allowed CPUs.
    assert(fast_cpu_mask(0x03,frequency)==0x03); // Never request excluded cores.
    assert(fast_cpu_mask(0x3c,frequency)==0x3c);
    frequency[3]=0;assert(fast_cpu_mask(0x3f,frequency)==0x3f); // Missing sysfs: retain scheduler choice.
    frequency.fill(2000000);assert(fast_cpu_mask(0x3f,frequency)==0x3f); // Homogeneous machine.
    assert(fast_cpu_mask(0,frequency)==0);
    frequency[63]=3000000;assert(fast_cpu_mask((uint64_t(1)<<63)|1,frequency)==(uint64_t(1)<<63));
    std::cout<<"HOTD allowed-core selection, measured clusters and neutral fallbacks passed\n";
}
