// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/start_policy.h"
#include <cassert>
#include <iostream>
using namespace hotd_model2;
int main(){
    const std::array<bool,2> up{false,false},tracked{true,true};
    GameState s;s.valid=s.ready=true;s.start_cost=2;
    NativeStartPolicy p;p.request(0,1,0,s);
    auto o=p.update(0,s,up,tracked,true);assert(o.coin_mask==1&&!o.start_mask);
    o=p.update(.16,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);
    // Two coins per credit: the first accepted coin changes only the native
    // remainder. A quick physical Start tap must remain pending through four.
    s.native_coins[0]=1;s.credit_remainder[0]=1;
    o=p.update(.20,s,up,tracked,true);assert(!o.start_mask);
    o=p.update(.31,s,up,tracked,true);assert(o.coin_mask==1&&!o.start_mask);
    s.native_coins[0]=2;s.credits[0]=1;s.credit_remainder[0]=0;
    o=p.update(.51,s,up,tracked,true);assert(!o.start_mask);
    o=p.update(.62,s,up,tracked,true);assert(o.coin_mask==1&&!o.start_mask);
    s.native_coins[0]=3;s.credit_remainder[0]=1;
    o=p.update(.82,s,up,tracked,true);assert(!o.start_mask);
    o=p.update(.93,s,up,tracked,true);assert(o.coin_mask==1&&!o.start_mask);
    s.native_coins[0]=4;s.credits[0]=2;s.credit_remainder[0]=0;
    o=p.update(1.14,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);
    o=p.update(1.24,s,up,tracked,true);assert(!o.coin_mask&&o.start_mask==1);
    // Counter acknowledgement without accounting progress is not permission
    // to emit more credit. Delayed native balance must arrive first.
    NativeStartPolicy delayed;s={};s.valid=s.ready=true;s.start_cost=1;delayed.request(10,1,0,s);
    assert(delayed.update(10,s,up,tracked,true).coin_mask==1);
    s.native_coins[0]=1;
    for(double t:{10.2,10.4,10.9,11.4}){o=delayed.update(t,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);}
    s.credits[0]=1;o=delayed.update(11.5,s,up,tracked,true);assert(o.start_mask==1&&!o.coin_mask);
    // No response produces one bounded pulse, never a timer-driven credit train.
    NativeStartPolicy lost;s={};s.valid=s.ready=true;lost.request(20,1,0,s);
    assert(lost.update(20,s,up,tracked,true).coin_mask==1);
    for(unsigned i=1;i<=300;i++){o=lost.update(20+i*.1,s,up,tracked,true);assert(!o.start_mask);if(i>=2)assert(!o.coin_mask);}
    // Bonus coinage and free play never buy unnecessary coins.
    NativeStartPolicy bonus;s={};s.valid=s.ready=true;s.start_cost=2;bonus.request(60,1,0,s);bonus.update(60,s,up,tracked,true);
    s.native_coins[0]=1;s.credits[0]=3;o=bonus.update(60.4,s,up,tracked,true);assert(o.start_mask==1&&!o.coin_mask);
    NativeStartPolicy free;s={};s.valid=s.ready=true;s.free_play=true;free.request(70,1,0,s);o=free.update(70,s,up,tracked,true);assert(o.start_mask==1&&!o.coin_mask);
    // Native prices can change while waiting. Start requires the current price.
    NativeStartPolicy repriced;s={};s.valid=s.ready=true;s.start_cost=2;repriced.request(80,1,0,s);repriced.update(80,s,up,tracked,true);
    s.native_coins[0]=1;s.credits[0]=2;s.start_cost=3;o=repriced.update(80.4,s,up,tracked,true);assert(!o.start_mask&&o.coin_mask==1);
    // Corruption, tracking loss, focus/menu suspension cancel pending intent.
    NativeStartPolicy cancel;s={};s.valid=s.ready=true;cancel.request(90,1,0,s);cancel.update(90,s,up,tracked,true);
    cancel.update(90.1,s,up,{true,false},true);s.credits[0]=1;o=cancel.update(90.5,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);
    NativeStartPolicy invalid;s.start_cost=99;invalid.request(100,1,0,s);o=invalid.update(100,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);
    NativeStartPolicy suspend;s.start_cost=1;s.credits={};suspend.request(110,1,0,s);suspend.update(110,s,up,tracked,true);suspend.update(110.1,s,up,tracked,false);s.credits[0]=1;o=suspend.update(111,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);
    // Shared-pool transactions wait for the first seat's native join/debit.
    // Its held Start retains P1 ownership while a later P2 request can proceed.
    NativeStartPolicy shared;s={};s.valid=s.ready=true;s.credits[0]=1;
    shared.request(120,1,0,s);shared.request(120.01,0,1,s);
    const std::array<bool,2> held{true,true};o=shared.update(120.02,s,held,tracked,true);assert(o.start_mask==1&&!o.coin_mask);
    o=shared.update(120.1,s,held,tracked,true);assert(o.start_mask==1&&!o.coin_mask);
    s.credits[0]=0;s.players[0].state=1;s.players[0].native_state=5;s.players[0].health=3;
    o=shared.update(120.31,s,held,tracked,true);assert(o.start_mask==1&&o.coin_mask==2);
    s.native_coins[1]=1;s.credits[0]=1;
    o=shared.update(120.65,s,held,tracked,true);assert(o.start_mask==3&&!o.coin_mask);
    // Bookkeeping reset/decrease is not an accepted coin acknowledgement.
    NativeStartPolicy reset;s={};s.valid=s.ready=true;s.start_cost=2;s.native_coins[0]=5;
    reset.request(130,1,0,s);reset.update(130,s,up,tracked,true);s.native_coins[0]=0;s.credit_remainder[0]=1;
    for(double t:{130.4,130.8,132.5}){o=reset.update(t,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);}
    // Even continuous fractional progress cannot produce an unbounded train.
    NativeStartPolicy cap;s={};s.valid=s.ready=true;s.start_cost=5;cap.request(140,1,0,s);unsigned coins=0;
    for(unsigned i=0;i<40;i++){
        const double t=140+i*.31;o=cap.update(t,s,up,tracked,true);if(o.coin_mask)++coins;assert(!o.start_mask);
        s.native_coins[0]++;s.credit_remainder[0]++;
        o=cap.update(t+.2,s,up,tracked,true);assert(!o.coin_mask&&!o.start_mask);
    }
    assert(coins==32);
    std::cout<<"Native credit acknowledgement, fractional/delayed coinage and bounded Start checks passed\n";
}
