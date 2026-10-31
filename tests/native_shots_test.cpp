// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/native_shots.h"
#include "../native/input_policy.h"
#include <cassert>
#include <iostream>
using namespace hotd_model2;
bool commit(NativeShotEvents& events,const GameState& s,unsigned player,uint64_t epoch){
    events.sample_trigger(player,false,0);
    events.sample_trigger(player,true,epoch);
    events.candidate(0x5bc0,player?0x51ef24:0x51ef08,0x100,0xff00);
    return events.observe(s,0x5c40,player?0x51ecd4:0x51ec08,2,0xffffffff);
}
int main() {
    GameState s;s.valid=s.ready=s.playing=true;
    for(auto& p:s.players){p.state=1;p.native_state=5;p.health=3;p.ammo=6;p.capacity=6;}
    NativeShotEvents events;
    assert(!events.observe(s,0x5c40,0x51ec08,2,0xffffffff));
    events.attach();
    // Captured original-program writes: provisional decrement/refund and reload
    // never commit a shot. The later shot flag write is the accepted event.
    assert(!events.observe(s,0x5c04,0x51ec40,0x500,0xff00));
    assert(!events.observe(s,0x5cc0,0x51ec40,0x600,0xff00));
    assert(!events.observe(s,0x5cec,0x51ec40,0x600,0xff00));
    assert(!events.observe(s,0x5a38,0x51ed0c,0x600,0xff00));
    assert(events.observe(s,0x5c40,0x51ec08,2,0xffffffff));
    s.players[0].ammo=0; // The final round is already consumed before commit.
    assert(events.observe(s,0x5c40,0x51ec08,2,0xffffffff));
    assert(events.observe(s,0x5c40,0x51ecd4,2,0xffffffff));
    assert(!events.observe(s,0x5acc,0x51ec08,0,0xffffffff));
    assert(!events.observe(s,0x5c40,0x51ec08,0,0xffffffff));
    assert(!events.observe(s,0x5c40,0x51ec08,2,0xffff));
    assert(!events.observe(s,0x5c40,0x51ec0c,2,0xffffffff));
    events.publish(s);assert(s.counting_shots&&s.players[0].shots==2&&s.players[1].shots==1);
    for(unsigned boundary=0;boundary<5;++boundary){
        auto closed=s;
        if(boundary==0)closed.valid=false;
        if(boundary==1)closed.ready=false;
        if(boundary==2)closed.playing=false;
        if(boundary==3)closed.service=true;
        if(boundary==4)closed.players[0].health=0;
        assert(!events.observe(closed,0x5c40,0x51ec08,2,0xffffffff));
    }
    events.publish(s);assert(s.players[0].shots==2&&s.players[1].shots==1);
    // Trigger edges and empty pulls do not create feedback; only native totals do.
    Hands hands;for(auto& h:hands){h.tracked=true;h.aim={.5,.5,true,true};}
    Settings settings;InputPolicy policy;
    policy.update(0,hands,s,settings,true,false);
    hands[0].trigger=true;
    auto input=policy.update(.1,hands,s,settings,true,false);assert(!input.shots[0]&&!input.shots[1]);
    commit(events,s,0,input.feedback_epoch[0]);events.publish(s);
    input=policy.update(.2,hands,s,settings,true,false);assert(input.shots[0]==1&&input.hand[0]==0&&!input.shots[1]);
    // Menu/focus/layout changes discard pending feedback rather than replaying it.
    for(unsigned boundary=0;boundary<3;++boundary){
        commit(events,s,1,input.feedback_epoch[1]);events.publish(s);
        if(boundary==2)settings.p1_hand=1;
        input=policy.update(1+boundary,hands,s,settings,boundary!=0,boundary==1);
        assert(!input.shots[0]&&!input.shots[1]);
        input=policy.update(1.1+boundary,hands,s,settings,true,false);assert(!input.shots[0]&&!input.shots[1]);
    }
    commit(events,s,1,input.feedback_epoch[1]);events.publish(s);
    input=policy.update(5,hands,s,settings,true,false);assert(input.shots[1]==1&&input.hand[1]==0);
    hands[0].tracked=false;
    commit(events,s,1,input.feedback_epoch[1]);events.publish(s);
    input=policy.update(5.1,hands,s,settings,true,false);assert(!input.shots[1]);
    hands[0].tracked=true;
    input=policy.update(5.2,hands,s,settings,true,false);assert(!input.shots[1]);
    hands[0].tracked=false;
    policy.update(5.3,hands,s,settings,true,false);
    // A native shot arriving between tracking loss and reacquisition is stale.
    commit(events,s,1,input.feedback_epoch[1]);events.publish(s);
    hands[0].tracked=true;
    input=policy.update(5.4,hands,s,settings,true,false);assert(!input.shots[1]);
    // A solo transfer must not send the previous hand's pending shot to its new owner.
    InputPolicy solo;s.players[1].state=0;settings.p1_hand=0;
    for(auto& h:hands)h.trigger=false;
    input=solo.update(6,hands,s,settings,true,false);
    hands[0].trigger=true;commit(events,s,0,input.feedback_epoch[0]);events.publish(s);
    input=solo.update(6.1,hands,s,settings,true,false);assert(input.hand[0]==0&&!input.shots[0]);
    commit(events,s,0,input.feedback_epoch[0]);events.publish(s);
    input=solo.update(6.2,hands,s,settings,true,false);assert(input.shots[0]==1&&input.hand[0]==0);
    // Native events can remain unpublished for several XR updates. Bind each
    // event to the input actually consumed by the engine, not desired XR layout.
    InputPolicy delayed;for(auto& h:hands)h.trigger=false;
    auto old_owner=delayed.update(7,hands,s,settings,true,false);assert(old_owner.hand[0]==1);
    commit(events,s,0,old_owner.feedback_epoch[0]);
    hands[0].trigger=true;
    input=delayed.update(7.1,hands,s,settings,true,false);assert(input.hand[0]==0);
    delayed.update(7.2,hands,s,settings,true,false);
    events.publish(s);input=delayed.update(7.3,hands,s,settings,true,false);assert(!input.shots[0]);
    // The same delayed publication must remain silent after tracking returns.
    old_owner=input;hands[0].tracked=false;delayed.update(7.4,hands,s,settings,true,false);
    commit(events,s,0,old_owner.feedback_epoch[0]);
    hands[0].tracked=true;delayed.update(7.5,hands,s,settings,true,false);
    delayed.update(7.6,hands,s,settings,true,false);
    events.publish(s);input=delayed.update(7.7,hands,s,settings,true,false);assert(!input.shots[0]);
    commit(events,s,0,input.feedback_epoch[0]);events.publish(s);
    input=delayed.update(7.8,hands,s,settings,true,false);assert(input.shots[0]==1);
    // Freeze the candidate's original edge token before a vblank can consume
    // another hand's input. Even a new sampled edge before commit cannot retag it.
    hands[0].trigger=hands[1].trigger=false;
    input=delayed.update(7.9,hands,s,settings,true,false);old_owner=input;
    events.sample_trigger(0,false,0);events.sample_trigger(0,true,old_owner.feedback_epoch[0]);
    events.candidate(0x5bc0,0x51ef08,0x100,0xff00);
    hands[0].trigger=false;hands[1].trigger=true;
    input=delayed.update(8,hands,s,settings,true,false);assert(input.hand[0]==1);
    delayed.update(8.1,hands,s,settings,true,false);
    events.sample_trigger(0,false,0);events.sample_trigger(0,true,input.feedback_epoch[0]);
    assert(events.observe(s,0x5c40,0x51ec08,2,0xffffffff));events.publish(s);
    input=delayed.update(8.2,hands,s,settings,true,false);assert(!input.shots[0]);
    commit(events,s,0,input.feedback_epoch[0]);events.publish(s);
    input=delayed.update(8.3,hands,s,settings,true,false);assert(input.shots[0]==1);
    events.detach();events.publish(s);assert(!s.counting_shots&&!s.players[0].shots&&!s.players[1].shots);
    events.attach();events.publish(s);assert(s.counting_shots&&!s.players[0].shots&&!s.players[1].shots);
    std::cout<<"HOTD committed native shots, rejected writes and feedback ownership passed\n";
}
