// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/cabinet_io.h"
#include <cassert>
#include <iostream>
using namespace hotd_model2;
int main() {
    Cabinet c;
    assert(c.x[0]==384 && c.x[1]==379 && c.y[0]==233 && c.y[1]==233);
    assert(in0(0xff,c)==0xff && in1(0xff,c)==0xff);
    c.coins=3;c.starts=3;c.test=true;c.service=true;c.trigger={true,true};
    assert(in0(0xff,c)==0xc0 && in0(0xa5,c)==0x80);
    assert(in1(0xff,c)==0xfc && in1(0x80,c)==0x80);
    c.x={0x123,0x234};c.y={0x345,0x156};
    const uint8_t bytes[]={0x45,3,0x23,1,0x56,1,0x34,2};
    for(unsigned i=0;i<8;i++)assert(serial_byte(i,c)==bytes[i]);
    for(unsigned p=0;p<2;p++) {
        const unsigned xmin=p?163:173,margin=p?21:21;
        assert(offscreen(p,xmin+margin,233));
        assert(!offscreen(p,xmin+margin+1,233));
        assert(offscreen(p,596-margin,233));
        assert(!offscreen(p,596-margin-1,233));
        assert(offscreen(p,380,101));assert(!offscreen(p,380,102));
        assert(offscreen(p,380,366));assert(!offscreen(p,380,365));
        assert(axis_from_normalized(0,p,true)==xmin);
        assert(axis_from_normalized(65535,p,true)==596);
        assert(axis_from_normalized(0,p,false)==87);
        assert(axis_from_normalized(65535,p,false)==380);
    }
    c={};assert(serial_byte(8,c)==0xfc && serial_byte(255,c)==0xfc);
    c.x[0]=173;assert(serial_byte(8,c)==0xfd);
    c.y[1]=380;assert(serial_byte(8,c)==0xff);
    c={};apply_reload(c,1);assert(c.trigger[1] && !c.trigger[0]);
    assert(offscreen(1,c.x[1],c.y[1]) && serial_byte(8,c)==0xfe);
    assert(in1(0xff,c)==0xfd);
    assert(offscreen(2,380,233));
    assert(calibrated_axis(0,100,900)==100&&calibrated_axis(65535,100,900)==900);
    assert(calibrated_axis(32768,100,900)==500);
    assert(calibrated_axis(32768,900,100)==0&&calibrated_axis(32768,0,1024)==0);
    std::cout<<"HOTD Model 2C cabinet checks passed\n";
}
