// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>
#include <string_view>
namespace hotd_model2 {
template<class Read32> bool native_gun_strobe(Read32 read) {
    // Original hotdo gun-reader45e00 fills the primary System24 window mask
    // with ones at18600, sets its cooldown to4, then clears it at18630 on
    // the next tick. Gun reads also precede reload/empty/refunded shots.
    // Check one original instruction before RAM; the complete fingerprint is
    // needed only for a possible strobe, not on every ordinary render frame.
    if(uint32_t(read(0x45e00))!=0x90a03000u)return false;
    if((uint32_t(read(0x51ef6c))&255)!=4 ||
       (uint32_t(read(0x51ee14))&65535)!=4 || uint32_t(read(0x51e258))!=0 ||
       (uint32_t(read(0x51ee34))&255)==4 || (uint32_t(read(0x51eee4))&0x20))return false;
    const uint32_t requests=uint32_t(read(0x5430b0));
    const auto eligible=[&](unsigned player) {
        if(((requests>>(player*8))&255)!=1)return false;
        const uint32_t state=uint32_t(read(0x51ec04+player*0xcc));
        // These are the original table's three enabled states. Never index
        // emulated ROM with an unchecked state or reuse another game's IDs.
        if(state!=5&&state!=8&&state!=11)return false;
        return ((uint32_t(read(0xa8784+state*8))>>16)&255)==1;
    };
    if(!eligible(0)&&!eligible(1))return false;
    constexpr uint32_t words[][2]={
        {0x45e04,0x0051ec04},{0x45e08,0x80a03994},{0x45e0c,0x000a8786},
        {0x45e14,0x80a03000},{0x45e18,0x005430b0},
        {0x45e20,0x90a03000},{0x45e24,0x0051ecd0},{0x45e28,0x80a03994},{0x45e2c,0x000a8786},
        {0x45e34,0x80a03000},{0x45e38,0x005430b1},
        {0x45e50,0x90a03000},{0x45e54,0x0051e258},
        {0x45e5c,0xc8a03000},{0x45e60,0x0051ee14},
        {0x45e70,0x80a03000},{0x45e74,0x0051ee34},
        {0x22d64,0x90803000},{0x22d68,0x0051eee4},{0x22d6c,0x59a8581f},{0x22d70,0x58854090},
        {0x45e8c,0x80a03000},{0x45e90,0x0051ef6c},
        {0x45e98,0x8c803000},{0x45e9c,0x0100c000},{0x45ea0,0x09fd2760},
        {0x45ec4,0x5ca81e04},{0x45ec8,0x82a83000},{0x45ecc,0x0051ef6c},
        {0x45efc,0x8c803000},{0x45f00,0x0100c000},{0x45f04,0x09fd272c},
        {0x18600,0x59381901},{0x1860c,0x59201901},{0x18614,0x8ca800bf},
        {0x1861c,0xb2241000},{0x18628,0x16fffff0},
        {0x18630,0x5c381e00},{0x1863c,0x5c201e00},{0x18644,0x8ca800bf},
        {0x1864c,0xb2241000},{0x18658,0x16fffff0}
    };
    for(const auto& word:words)if(uint32_t(read(word[0]))!=word[1])return false;
    for(uint32_t at=0x0100c000;at<0x0100cc00;at+=4)
        if(uint32_t(read(at))!=0xffffffffu)return false;
    return true;
}
template<class Read32> const uint16_t* native_gun_window_mask(bool vr,std::string_view system,bool keep_strobe,Read32 read) {
    // This is a per-draw decision. No emulated RAM or persistent mask pointer
    // is changed; the next frame automatically returns to its own native mask.
    static constexpr uint16_t clear_mask[0x800]{};
    return vr&&system=="hotdo"&&!keep_strobe&&native_gun_strobe(read)?clear_mask:nullptr;
}
}
