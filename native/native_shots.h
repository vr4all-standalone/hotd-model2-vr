// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game_state.h"
namespace hotd_model2 {
// Original hotdo's committed-shot flag store, after the blocked-shot branch.
// The earlier ammo decrement can be refunded and is not an accepted event.
// Used only by the emulation thread; publish copies totals into locked state.
class NativeShotEvents {
    bool attached_{};
    std::array<uint32_t,2> counts_{};
    std::array<uint64_t,2> epochs_{};
    std::array<uint32_t,2> feedback_counts_{};
    std::array<bool,2> trigger_down_{};
    std::array<uint64_t,2> trigger_epoch_{},candidate_epoch_{};
public:
    void attach(){counts_={};epochs_={};feedback_counts_={};trigger_down_={};trigger_epoch_={};candidate_epoch_={};attached_=true;}
    void detach(){attached_=false;}
    void sample_trigger(unsigned player,bool down,uint64_t epoch){
        if(player>1||!attached_)return;
        if(down&&!trigger_down_[player])trigger_epoch_[player]=epoch;
        trigger_down_[player]=down;
    }
    void candidate(uint32_t ip,uint32_t address,uint32_t data,uint32_t mask){
        if(!attached_||ip!=0x5bc0||mask!=0xff00)return;
        const int player=address==0x51ef08?0:address==0x51ef24?1:-1;
        if(player>=0)candidate_epoch_[player]=(data&0xff00)?trigger_epoch_[player]:0;
    }
    bool observe(const GameState& state,uint32_t ip,uint32_t address,uint32_t data,uint32_t mask) {
        if(!attached_||ip!=0x5c40||mask!=0xffffffffu||(data&2)==0||
           !state.valid||!state.ready||!state.playing||state.service)return false;
        const int player=address==0x51ec08?0:address==0x51ecd4?1:-1;
        if(player<0||!state.players[player].alive())return false;
        const auto epoch=candidate_epoch_[player];candidate_epoch_[player]=0;
        ++counts_[player];
        if(epochs_[player]!=epoch){epochs_[player]=epoch;feedback_counts_[player]=0;}
        ++feedback_counts_[player];return true;
    }
    void publish(GameState& state)const {
        state.counting_shots=attached_&&state.valid;
        for(unsigned p=0;p<2;++p){
            state.players[p].shots=state.counting_shots?counts_[p]:0;
            state.players[p].feedback_epoch=state.counting_shots?epochs_[p]:0;
            state.players[p].feedback_shots=state.counting_shots?feedback_counts_[p]:0;
        }
    }
};
}
