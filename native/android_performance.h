// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cstdint>
#ifdef __ANDROID__
#include <sched.h>
#include <cerrno>
#include <fstream>
#include <string>
#endif
namespace hotd_model2 {
// Keep all fastest cores already available to this thread. Missing capacity
// information leaves the scheduler's existing choice intact.
inline uint64_t fast_cpu_mask(uint64_t allowed,const std::array<uint64_t,64>& frequency){
    uint64_t maximum=0,result=0;
    for(unsigned cpu=0;cpu<64;++cpu)if(allowed&(uint64_t(1)<<cpu)){
        if(!frequency[cpu])return allowed;
        if(frequency[cpu]>maximum){maximum=frequency[cpu];result=0;}
        if(frequency[cpu]==maximum)result|=uint64_t(1)<<cpu;
    }
    return result?result:allowed;
}
struct NativeCpuPlacement{uint64_t allowed{},selected{};bool applied{};int error{};};
inline NativeCpuPlacement prefer_fast_native_cpus(){
    NativeCpuPlacement out;
#ifdef __ANDROID__
    cpu_set_t available;
    if(sched_getaffinity(0,sizeof(available),&available)!=0){out.error=errno;return out;}
    for(unsigned cpu=64;cpu<CPU_SETSIZE;++cpu)if(CPU_ISSET(cpu,&available))return out;
    std::array<uint64_t,64> frequency{};
    for(unsigned cpu=0;cpu<64;++cpu)if(CPU_ISSET(cpu,&available)){
        out.allowed|=uint64_t(1)<<cpu;
        std::ifstream input("/sys/devices/system/cpu/cpu"+std::to_string(cpu)+"/cpufreq/cpuinfo_max_freq");
        input>>frequency[cpu];
    }
    out.selected=fast_cpu_mask(out.allowed,frequency);
    if(out.selected&&out.selected!=out.allowed){
        cpu_set_t selected;CPU_ZERO(&selected);
        for(unsigned cpu=0;cpu<64;++cpu)if(out.selected&(uint64_t(1)<<cpu))CPU_SET(cpu,&selected);
        if(sched_setaffinity(0,sizeof(selected),&selected)==0)out.applied=true;
        else out.error=errno;
    }
#endif
    return out;
}
}
