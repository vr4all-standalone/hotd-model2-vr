// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/input_policy.h"
#include <cassert>
#include <iostream>
using namespace hotd_model2;
int main(){for(int layout:{0,1}){
    Settings settings;settings.p1_hand=layout;InputPolicy policy;Hands h;
    for(auto& x:h){x.tracked=true;x.aim={.5,.5,true,true};}
    GameState s;s.valid=s.ready=s.playing=true;s.credits={5,5};
    for(auto& p:s.players){p.state=1;p.native_state=5;p.health=3;}
    policy.update(0,h,s,settings,true,false);for(auto& x:h)x.trigger=true;
    auto out=policy.update(.1,h,s,settings,true,false);assert(out.hand[1]==1-layout&&out.guns[1].trigger);
    s.players[0]={};s.players[0].native_state=9;
    out=policy.update(.2,h,s,settings,true,false);assert(out.hand[0]==-1&&out.hand[1]==1-layout&&!out.guns[1].trigger);
    out=policy.update(.3,h,s,settings,true,false);assert(out.hand[1]==1-layout&&!out.guns[1].trigger);
    for(auto& x:h)x.trigger=false;policy.update(.4,h,s,settings,true,false);
    h[1-layout].trigger=true;out=policy.update(.5,h,s,settings,true,false);assert(out.guns[1].trigger&&!out.guns[0].trigger);
    // The remaining native P2 seat keeps its controller even if the saved
    // dual layout changes; its own Start must continue that logical player.
    settings.p1_hand=1-layout;for(auto& x:h)x.trigger=false;
    s.players[1].state=2;s.players[1].native_state=4;s.players[1].health=0;
    policy.update(.6,h,s,settings,true,false);h[1-layout].start=true;
    out=policy.update(.7,h,s,settings,true,false);assert(out.hand[1]==1-layout&&out.start_mask==2&&!out.coin_mask);
    out=policy.update(.8,h,s,settings,true,false);assert(out.start_mask==2);
    h[0].tracked=h[1].tracked=false;out=policy.update(.9,h,s,settings,true,false);assert(!out.start_mask&&!out.coin_mask&&!out.guns[1].trigger);
    h[1-layout].tracked=true;out=policy.update(1,h,s,settings,true,false);assert(!out.start_mask&&!out.coin_mask);
    h[1-layout].start=false;policy.update(1.1,h,s,settings,true,false);h[1-layout].start=true;
    out=policy.update(1.2,h,s,settings,true,false);assert(out.start_mask==2&&!out.coin_mask);
    // A sole tracked replacement hand can transfer P2 and press Start on the
    // same fresh edge. Resolve the hand before selecting the native seat.
    InputPolicy transfer;settings.p1_hand=layout;Hands t;
    for(auto& x:t){x.tracked=true;x.aim={.5,.5,true,true};}
    GameState seat;s=seat;s.valid=s.ready=s.playing=true;s.credits={5,5};
    for(auto& p:s.players){p.state=1;p.native_state=5;p.health=3;}
    transfer.update(10,t,s,settings,true,false);s.players[0]={};s.players[0].native_state=9;
    transfer.update(10.1,t,s,settings,true,false);s.players[1].state=2;s.players[1].native_state=4;s.players[1].health=0;
    transfer.update(10.2,t,s,settings,true,false);t[1-layout].tracked=false;t[layout].trigger=t[layout].start=true;
    out=transfer.update(10.3,t,s,settings,true,false);assert(out.hand[1]==layout&&out.start_mask==2&&!out.coin_mask);
}std::cout<<"P2-only native identity, remaining hand and continue guards passed in both layouts\n";}
