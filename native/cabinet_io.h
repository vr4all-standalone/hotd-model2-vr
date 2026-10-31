// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cstdint>
namespace hotd_model2 {
// MAME 0.289 model2.cpp hotd ports + lightgun_mux/data/offscreen_r.
struct Cabinet {
    std::array<uint16_t,2> x{384,379},y{233,233};
    std::array<bool,2> trigger{};
    uint8_t coins{},starts{};
    bool test{},service{};
};
inline uint8_t in0(uint8_t native,const Cabinet& c) {
    const unsigned active=(c.coins&3)|((c.starts&3)<<4)|(c.test?4:0)|(c.service?8:0);
    return uint8_t(native&~active);
}
inline uint8_t in1(uint8_t native,const Cabinet& c) {
    return uint8_t(native&~((c.trigger[0]?1:0)|(c.trigger[1]?2:0)));
}
inline bool offscreen(unsigned player,uint16_t x,uint16_t y) {
    if(player>1)return true;
    const unsigned xmin=player?163:173,xmax=596,ymin=87,ymax=380;
    const unsigned bx=(xmax-xmin)*5/100,by=(ymax-ymin)*5/100;
    return x<=xmin+bx||x>=xmax-bx||y<=ymin+by||y>=ymax-by;
}
inline uint8_t serial_byte(unsigned mux,const Cabinet& c) {
    if(mux>=8)return uint8_t(0xfc|(offscreen(0,c.x[0],c.y[0])?1:0)|(offscreen(1,c.x[1],c.y[1])?2:0));
    const uint16_t values[]={c.y[0],c.x[0],c.y[1],c.x[1]};
    return uint8_t(values[mux/2]>>((mux&1)*8));
}
// Default cabinet range mapping only. Native HOTD saved calibration inversion
// remains unavailable until its actual program and NVRAM have been measured.
inline uint16_t axis_from_normalized(uint16_t value,unsigned player,bool horizontal) {
    if(player>1)return 0;
    const unsigned lo=horizontal?(player?163:173):87,hi=horizontal?596:380;
    return uint16_t(lo+(uint64_t(value)*(hi-lo)+32767)/65535);
}
inline uint16_t calibrated_axis(uint16_t value,uint32_t lo,uint32_t hi) {
    if(lo>=hi||hi>1023)return 0;
    return uint16_t(lo+(uint64_t(value)*(hi-lo)+32767)/65535);
}
inline void apply_reload(Cabinet& c,unsigned player) {
    if(player>1)return;
    c.x[player]=0;c.y[player]=0;c.trigger[player]=true;
}
}
