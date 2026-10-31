// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game_state.h"
#include <array>
#include <cmath>
#include <cstdint>
namespace hotd_model2 {
struct StartOutput {uint8_t coin_mask{},start_mask{};};
// Credits are native accounting results, not a count of cabinet coin pulses.
// An accepted fractional coin advances bookkeeping/remainder before the full
// balance changes. Missing native acknowledgement never starts a credit train.
class NativeStartPolicy {
    static constexpr double CoinHigh=.15,CoinLow=.15,CoinTimeout=2,RequestTimeout=20,StartHigh=.25,StartAckTimeout=1;
    static constexpr unsigned MaxCoins=32;
    struct Accounting {
        std::array<uint32_t,2> credits{},remainder{};
        std::array<uint16_t,2> fraction{};
        bool operator==(const Accounting&)const=default;
    };
    struct Pending {
        bool active{},waiting_coin{},delivering{},joined_before{},acknowledged{};
        int hand{-1};unsigned sent{};uint32_t native_before{};
        double began{},coin_at{-100},next_coin{},started{};
        uint16_t coin_before{};Accounting accounting;
    };
    std::array<Pending,2> pending_{};
    double last_{-1};
    static Accounting accounting(const GameState& s){return {s.credits,s.credit_remainder,s.coin_fraction};}
    static unsigned price(const GameState& s,unsigned player){return s.free_play||s.players[player].alive()?0:s.players[player].native_state==4?s.continue_cost:s.start_cost;}
    static bool usable(const GameState& s){return s.valid&&s.ready&&!s.service&&s.start_cost<=5&&s.continue_cost<=5;}
public:
    void suspend(){pending_={};last_=-1;}
    void cancel_hand(int hand){for(auto& p:pending_)if(p.active&&p.hand==hand)p={};}
    void cancel_waiting(){for(auto& p:pending_)if(!p.delivering)p={};}
    void request(double now,int hand,unsigned player,const GameState& s){
        if(!std::isfinite(now)||hand<0||hand>1||player>1||!usable(s)||pending_[player].active)return;
        auto& p=pending_[player];p.active=true;p.hand=hand;p.began=p.next_coin=now;
        p.joined_before=s.players[player].joined();p.native_before=s.players[player].native_state;
    }
    StartOutput update(double now,const GameState& s,const std::array<bool,2>& held,const std::array<bool,2>& tracked,bool enabled){
        StartOutput out;
        if(!enabled||!usable(s)||!std::isfinite(now)||(last_>=0&&now<last_)){suspend();return out;}
        last_=now;
        for(auto& p:pending_)if(p.active&&(!tracked[p.hand]||now-p.began>RequestTimeout))p={};
        // A held native Start retains its original seat. Only an acknowledged
        // join/continue (after the original debit) releases shared funding for
        // the next seat; merely emitting Start never spends a host reservation.
        bool reserved=false;
        for(int i=0;i<2;i++){
            auto& p=pending_[i];if(!p.active||!p.delivering)continue;
            p.acknowledged|=s.players[i].joined()&&(!p.joined_before||(p.native_before==4&&s.players[i].native_state!=4));
            const double elapsed=now-p.started;
            if(elapsed<StartHigh||held[p.hand])out.start_mask|=uint8_t(1u<<i);
            if(elapsed>=StartHigh&&!held[p.hand]&&(p.acknowledged||elapsed>=StartAckTimeout))p={};
            if(p.active&&!p.acknowledged)reserved=true;
        }
        if(reserved)return out;
        int player=-1;
        for(int i=0;i<2;i++)if(pending_[i].active&&!pending_[i].delivering&&(player<0||pending_[i].began<pending_[player].began))player=i;
        if(player<0)return out;
        auto& p=pending_[player];const uint8_t bit=uint8_t(1u<<player);
        const auto available=s.credits[s.shared_credits?0:player];
        const bool funded=available>=price(s,unsigned(player));
        const bool coin_high=p.sent&&now<p.coin_at+CoinHigh;
        if(coin_high){out.coin_mask=bit;return out;}
        if(funded){
            // Finish the prior coin release interval before presenting Start.
            if(now<p.next_coin)return out;
            p.delivering=true;p.started=now;p.acknowledged=s.free_play||s.players[player].alive();out.start_mask|=bit;return out;
        }
        if(p.waiting_coin){
            // A reset/decrease or jump is not proof of this one pulse. On an
            // ambiguous bookkeeping wrap, cancel safely instead of buying more.
            if(s.native_coins[player]<p.coin_before){p={};return out;}
            const bool accepted=unsigned(s.native_coins[player])-p.coin_before==1;
            const bool accounted=accounting(s)!=p.accounting;
            if(accepted&&accounted)p.waiting_coin=false;
            else {if(now-p.coin_at>=CoinTimeout)p={};return out;}
        }
        if(now<p.next_coin)return out;
        if(p.sent>=MaxCoins){p={};return out;}
        p.waiting_coin=true;p.coin_before=s.native_coins[player];p.accounting=accounting(s);
        p.coin_at=now;p.next_coin=now+CoinHigh+CoinLow;++p.sent;out.coin_mask=bit;
        return out;
    }
};
}
