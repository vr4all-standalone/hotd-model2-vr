// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/input_policy.h"
#include "../native/settings_io.h"
#include "../native/weapon_choice.h"
#include <cassert>
#include <sstream>
#include <iostream>
using namespace hotd_model2;
int main(){
    Settings s;assert(s.gun==AutoOriginalGun&&!s.wider_view&&!s.game_zoom&&s.p1_hand==0);
    assert(selectable_gun_choice(8)&&!selectable_gun_choice(9));
    Hands h;for(auto& x:h){x.tracked=true;x.aim={.5,.5,true,true};}
    InputPolicy policy;GameState unknown;
    for(int tick=0;tick<10;tick++){
        h[0].start=h[1].start=tick&1;h[0].trigger=h[1].trigger=tick&1;
        auto out=policy.update(tick*.2,h,unknown,s,true,false);
        assert(!out.coin_mask&&!out.start_mask&&!out.guns[0].trigger&&!out.guns[1].trigger&&!out.shots[0]&&!out.shots[1]);
    }
    GameState verified;verified.valid=true;verified.ready=true;
    auto input=policy.update(3,h,verified,s,true,false);assert(!input.coin_mask&&!input.guns[0].trigger);
    for(auto& x:h){x.start=false;x.trigger=false;}
    policy.update(3.1,h,verified,s,true,false);
    h[1].start=true;input=policy.update(3.2,h,verified,s,true,false);assert(input.coin_mask==1&&!input.start_mask);
    verified.credits[0]=1;verified.native_coins[0]=1;
    input=policy.update(3.35,h,verified,s,true,false);assert(!input.start_mask);
    input=policy.update(3.51,h,verified,s,true,false);assert(input.start_mask==1);
    policy.update(3.6,h,verified,s,true,true);
    input=policy.update(3.7,h,verified,s,true,false);assert(!input.start_mask&&!input.coin_mask);
    h[1].start=false;policy.update(3.8,h,verified,s,true,false);
    h[1].trigger=true;input=policy.update(3.9,h,verified,s,true,false);assert(input.guns[0].trigger);
    policy.update(4,h,verified,s,false,false);
    input=policy.update(4.1,h,verified,s,true,false);assert(!input.guns[0].trigger);
    h[1].trigger=false;policy.update(4.2,h,verified,s,true,false);
    verified.players[0].state=verified.players[1].state=1;s.p1_hand=0;
    policy.update(4.3,h,verified,s,true,false);
    h[0].trigger=h[1].trigger=true;input=policy.update(4.4,h,verified,s,true,false);
    assert(input.hand[0]==0&&input.hand[1]==1&&input.guns[0].trigger&&input.guns[1].trigger);
    s.p1_hand=1;input=policy.update(4.5,h,verified,s,true,false);assert(!input.guns[0].trigger&&!input.guns[1].trigger);
    for(auto& x:h)x.trigger=false;policy.update(4.6,h,verified,s,true,false);
    for(auto& x:h)x.trigger=true;input=policy.update(4.7,h,verified,s,true,false);
    assert(input.hand[0]==1&&input.hand[1]==0&&input.guns[0].trigger&&input.guns[1].trigger);
    s.gun=1;s.gun_angle=19;s.panel_distance=2.4;s.haptics=.35;s.reload=ReloadMode::Button;
    std::stringstream saved;write_settings(saved,s);const auto restored=read_settings(saved);
    assert(restored.gun==1&&restored.p1_hand==1&&restored.gun_angle==19&&restored.panel_distance==2.4f&&restored.haptics==.35f&&restored.reload==ReloadMode::ShootOutside);
    std::stringstream malicious("version=6\ngun=8\nwider_view=1\ngun_angle_zero_version=1\ngun_angle=nan\np1_hand=9\nhaptics=2\n");
    const auto clean=read_settings(malicious);assert(clean.gun==AutoOriginalGun&&!clean.wider_view&&clean.p1_hand==0&&clean.gun_angle==0&&clean.haptics==.65f);
    InputPolicy two_credit;GameState title;title.valid=title.ready=true;title.start_cost=2;
    for(auto& x:h){x.start=false;x.trigger=false;}
    two_credit.update(10,h,title,s,true,false);h[1].start=true;
    input=two_credit.update(10.1,h,title,s,true,false);assert(input.coin_mask==1&&!input.start_mask);
    input=two_credit.update(10.25,h,title,s,true,false);assert(!input.coin_mask&&!input.start_mask);
    title.native_coins[0]=1;title.credits[0]=1;
    input=two_credit.update(10.31,h,title,s,true,false);assert(!input.coin_mask&&!input.start_mask);
    input=two_credit.update(10.41,h,title,s,true,false);assert(input.coin_mask==1&&!input.start_mask);
    title.native_coins[0]=2;title.credits[0]=2;
    input=two_credit.update(10.61,h,title,s,true,false);assert(!input.start_mask&&!input.coin_mask);
    input=two_credit.update(10.72,h,title,s,true,false);assert(input.start_mask==1&&!input.coin_mask);
    input=two_credit.update(10.8,h,title,s,true,false);assert(input.start_mask==1&&!input.coin_mask);
    InputPolicy balance;title.credits[0]=2;h[1].start=false;balance.update(20,h,title,s,true,false);h[1].start=true;
    input=balance.update(20.1,h,title,s,true,false);assert(!input.coin_mask);
    input=balance.update(20.25,h,title,s,true,false);assert(input.start_mask==1);
    InputPolicy free;title.free_play=true;title.credits[0]=0;h[1].start=false;free.update(30,h,title,s,true,false);h[1].start=true;
    input=free.update(30.1,h,title,s,true,false);assert(!input.coin_mask);
    input=free.update(30.25,h,title,s,true,false);assert(input.start_mask==1);
    std::cout<<"HOTD fresh-input, native acknowledgement boundary and persistence checks passed\n";
}
