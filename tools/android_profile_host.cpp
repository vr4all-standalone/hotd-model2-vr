// SPDX-License-Identifier: GPL-2.0-or-later
// Diagnostic synthetic hands through production input policy. No graphics/XR.
#include "../native/shared_runtime.h"
#include <thread>
#include <atomic>
#include <chrono>
namespace {std::thread worker;std::atomic<bool> stop{};}
extern "C" __attribute__((visibility("default"))) bool hotd_model2_xr_start(hotd_model2::SharedRuntime* shared){
    using namespace hotd_model2;
    if(!shared||shared->abi!=SharedRuntime::Abi||worker.joinable())return false;
    stop=false;shared->host_ready=true;
    worker=std::thread([shared]{
        InputPolicy policy;Settings settings;
        while(!stop){
            GameState state;{std::lock_guard lock(shared->mutex);state=shared->state;}
            auto frame=shared->scene.load();double now=frame?frame->time:0;
            Hands hands;for(auto& hand:hands){hand.tracked=true;hand.aim={.5,.5,true,true};hand.trigger=std::fmod(now,.8)<.15;hand.pump=now>30&&std::fmod(now,5.)<.2;}
            if(state.valid&&state.ready){
                hands[1].start=(!state.players[0].joined()||state.players[0].state==2)&&std::fmod(now,2.)<.35;
                hands[0].start=state.players[0].joined()&&(!state.players[1].joined()||state.players[1].state==2)&&std::fmod(now,2.)<.35;
            }
            auto input=policy.update(now,hands,state,settings,true,false);
            {std::lock_guard lock(shared->mutex);shared->input=input;}
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    });return true;
}
extern "C" __attribute__((visibility("default"))) void hotd_model2_xr_stop(){stop=true;if(worker.joinable())worker.join();}
extern "C" __attribute__((visibility("default"))) void hotd_model2_xr_android_init(void*,void*){}
