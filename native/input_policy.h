// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game_state.h"
#include "reticle_mode.h"
#include "start_policy.h"
#include <algorithm>
#include <array>
#include <cmath>
namespace hotd_model2 {
struct Aim { double u{.5},v{.5};bool valid{},inside{}; };
struct HandInput { bool tracked{},trigger{},start{},pump{}; Aim aim; };
using Hands=std::array<HandInput,2>; // Left, right
enum class ReloadMode { Button, ShootOutside, PointOutside };
enum class BackgroundMode {Original,Depth3D};
inline constexpr std::array<float,4> HapticLevels{0,.35f,.65f,1};
inline int haptic_level(float value){int best=0;for(int i=1;i<4;i++)if(std::abs(value-HapticLevels[i])<std::abs(value-HapticLevels[best]))best=i;return best;}
inline constexpr float DefaultCrosshairMaxSize = .60f;
struct Settings {
    ReticleMode reticle_mode{ReticleMode::Custom};
    float reticle_min_size{.22f},reticle_max_size{DefaultCrosshairMaxSize},reticle_shrink_distance{6.7f};
    int p1_hand{0}; ReloadMode reload{ReloadMode::ShootOutside};
    float world_scale{0.10f},panel_distance{1.8f},hud_size{1.f},gun_angle{},gun_scale{1},haptics{.65f};
    float view_size{1.f}; // Camera framing only; tracked poses and gun/2D HUD sizes stay physical.
    bool laser{true}; int gun{8}; // Verified original scene pistol; Arcade while resources load.
    BackgroundMode background{BackgroundMode::Original};
    bool wider_view{false};
    bool game_zoom{false};
    float view_fov{110};
    int laser_width{1}; // Normal, 5x, 10x
};
struct CabinetGun { uint16_t x{32768},y{32768};bool trigger{},pump{}; };
struct CabinetInput {
    std::array<CabinetGun,2> guns;
    std::array<int,2> hand{1,-1};
    std::array<uint32_t,2> shots{};
    std::array<uint64_t,2> feedback_epoch{};
    uint8_t coin_mask{},start_mask{};
};
constexpr int start_player_for_hand(int hand,int joined,int p1_hand,int solo_hand){
    const int active=joined==2?1:0;
    return joined==0?0:joined==3?(hand==p1_hand?0:1):(hand==solo_hand?active:1-active);
}
// Runs at native vblank, so a held-trigger handoff gets a complete release
// frame even when the independently paced XR thread updates faster than MAME.
class CabinetLatch {
    CabinetInput previous_;
public:
    CabinetInput update(const CabinetInput& desired) {
        auto out=desired;
        for(int p=0;p<2;p++)if(previous_.hand[p]!=desired.hand[p]&&previous_.guns[p].trigger&&desired.guns[p].trigger)out.guns[p].trigger=false;
        for(int p=0;p<2;p++)if(desired.guns[p].pump&&previous_.guns[p].trigger){out.guns[p].pump=false;out.guns[p].trigger=false;}
        previous_=out;return out;
    }
};
class OperatorLatch {
    std::array<unsigned,4> ticks_{};
public:
    uint16_t update(uint16_t pending) {
        uint16_t result=0;
        constexpr uint16_t buttons[]={4,8,0x10,0x20};
        for(int i=0;i<4;i++) {
            if(pending&buttons[i])ticks_[i]=6; // Measured operator debounce needs a sustained press.
            if(ticks_[i]){result|=buttons[i];--ticks_[i];}
        }
        return result;
    }
};
class InputPolicy {
    Hands previous_{};
    bool initialized_{},was_enabled_{},native_start_ready_{};
    int previous_layout_{},previous_join_{},solo_hand_{1};
    std::array<bool,2> blocked_{true,true},pump_blocked_{true,true},pull_outside_{},outside_{};
    std::array<bool,2> start_waiting_{true,true};
    NativeStartPolicy starts_;
    GameState previous_state_;
    std::array<int,2> previous_feedback_hand_{-1,-1};
    std::array<uint64_t,2> feedback_epoch_{1,1};
    std::array<bool,2> feedback_enabled_{};
    std::array<double,2> reload_until_{};
public:
    void suspend(){initialized_=false;was_enabled_=false;blocked_={true,true};pump_blocked_={true,true};start_waiting_={true,true};starts_.suspend();}
    CabinetInput update(double now,const Hands& hands,const GameState& state,
                        const Settings& settings,bool focused,bool menu) {
        CabinetInput out;
        int joined=state.valid?((state.players[0].joined()?1:0)|(state.players[1].joined()?2:0)):0;
        native_start_ready_=state.valid&&state.ready;
        const bool dual=joined==3,enabled=focused&&!menu&&native_start_ready_;
        const int p1hand=settings.p1_hand==1?1:0;
        if(previous_join_==3&&joined==2)solo_hand_=1-previous_layout_;
        else if(previous_join_==3&&joined==1)solo_hand_=previous_layout_;
        if(initialized_&&p1hand!=previous_layout_){starts_.cancel_waiting();start_waiting_={true,true};}
        bool transition=!initialized_ || !enabled || enabled!=was_enabled_ || joined!=previous_join_ || p1hand!=previous_layout_;
        if(transition) { blocked_={true,true};pump_blocked_={true,true};pull_outside_={};outside_={};reload_until_={}; }
        for(int h=0;h<2;h++) {
            const auto& in=hands[h];
            if(!in.trigger) { blocked_[h]=false;pull_outside_[h]=false; }
            if(!in.pump)pump_blocked_[h]=false;
            if(!in.tracked){blocked_[h]=true;pump_blocked_[h]=true;}
        }
        // Keep the remaining native seat's hand after dual play. Resolve an
        // intentional solo transfer before a simultaneous fresh Start edge.
        if(dual)solo_hand_=p1hand;
        else if(enabled) {
            const int other=1-solo_hand_;
            const bool was_pulled=previous_[solo_hand_].tracked&&previous_[solo_hand_].trigger;
            const bool pulled=hands[solo_hand_].tracked&&hands[solo_hand_].trigger;
            if(!was_pulled&&hands[other].tracked&&(!hands[solo_hand_].tracked||
                (hands[other].trigger&&!pulled&&!blocked_[other])))solo_hand_=other;
        }
        if(joined==0&&enabled&&was_enabled_&&p1hand==previous_layout_){
            const auto first=[&](int h){return hands[h].tracked&&!start_waiting_[h]&&hands[h].start&&!previous_[h].start;};
            if(first(0)||first(1))solo_hand_=first(0)&&first(1)?p1hand:first(0)?0:1;
        }
        for(int h=0;h<2;h++) {
            const auto& in=hands[h];
            // Sample physical Start even while the game cannot receive it.
            // Loading/menu/focus/tracking holds must be released before reuse.
            const bool start_ready=enabled&&native_start_ready_&&in.tracked;
            if(!start_ready){start_waiting_[h]=true;starts_.cancel_hand(h);}
            else if(!in.start)start_waiting_[h]=false;
            if(start_ready&&!start_waiting_[h]&&in.start&&!previous_[h].start&&(joined||h==solo_hand_)) {
                const int owner=start_player_for_hand(h,joined,p1hand,solo_hand_);
                // Solo P2 Start is a fresh native join request, never a queued
                // boot press. The native acknowledgement alone changes layout.
                if((joined&(1<<owner))||dual||(owner==0&&joined==0)||
                    (joined&&hands[0].tracked&&hands[1].tracked))starts_.request(now,h,unsigned(owner),state);
            }
            if(in.trigger&&!previous_[h].trigger) pull_outside_[h]=!in.aim.inside;
        }
        if(dual) out.hand={p1hand,1-p1hand};
        else out.hand=joined==2?std::array<int,2>{-1,solo_hand_}:std::array<int,2>{solo_hand_,-1};
        for(int p=0;p<2;p++) {
            const int h=out.hand[p];
            if(h<0)continue;
            const auto& in=hands[h];auto& gun=out.guns[p];
            const bool aim=in.tracked&&in.aim.valid&&std::isfinite(in.aim.u)&&std::isfinite(in.aim.v);
            if(aim) { gun.x=uint16_t(std::lround(std::clamp(in.aim.u,0.,1.)*65535));gun.y=uint16_t(std::lround(std::clamp(in.aim.v,0.,1.)*65535)); }
            const bool edge=in.trigger&&!previous_[h].trigger;
            const bool off=aim&&!in.aim.inside;
            if(enabled) {
                for(int pump_hand=0;pump_hand<2;pump_hand++)if((!dual||pump_hand==h)&&hands[pump_hand].tracked&&hands[pump_hand].pump&&!pump_blocked_[pump_hand])gun.pump=true;
            }
            if(enabled && aim && !blocked_[h]) {
                gun.trigger=in.trigger&&in.aim.inside&&!pull_outside_[h];
                gun.pump=gun.pump || (settings.reload==ReloadMode::ShootOutside && off&&edge) ||
                    (settings.reload==ReloadMode::PointOutside && off&&!outside_[h]);
            }
            if(gun.pump)reload_until_[p]=now+.12;
            gun.pump=enabled&&now<reload_until_[p];
            if(gun.pump)gun.trigger=false;
            outside_[h]=off;
        }
        const auto start=starts_.update(now,state,{hands[0].start,hands[1].start},
            {hands[0].tracked,hands[1].tracked},enabled);
        out.coin_mask=start.coin_mask;out.start_mask=start.start_mask;
        // Invalidate delayed native events across focus/menu/tracking/ownership
        // boundaries. The emulation thread echoes the token it actually used.
        for(int p=0;p<2;++p){
            const int hand=out.hand[p];
            const bool owner=enabled&&hand>=0&&hands[hand].tracked;
            if(transition||owner!=feedback_enabled_[p]||hand!=previous_feedback_hand_[p])++feedback_epoch_[p];
            out.feedback_epoch[p]=owner?feedback_epoch_[p]:0;
            feedback_enabled_[p]=owner;
        }
        if(enabled && !transition && state.valid && previous_state_.valid && state.counting_shots && previous_state_.counting_shots) {
            for(int p=0;p<2;p++) {
                const auto epoch=out.feedback_epoch[p];
                const auto& current=state.players[p];const auto& previous=previous_state_.players[p];
                auto count=current.feedback_shots,before=previous.feedback_epoch==epoch?previous.feedback_shots:0;
                const int hand=out.hand[p];
                if(epoch && current.feedback_epoch==epoch && count>=before && count-before<=64 && current.alive() &&
                   hand>=0 && hands[hand].tracked && previous_[hand].tracked && hand==previous_feedback_hand_[p]) out.shots[p]=count-before;
            }
        }
        previous_=hands;previous_state_=state;previous_feedback_hand_=out.hand;previous_layout_=p1hand;previous_join_=joined;
        was_enabled_=enabled;initialized_=true;
        return out;
    }
};
}
