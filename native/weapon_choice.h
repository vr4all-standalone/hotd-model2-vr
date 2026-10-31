// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game_state.h"
namespace hotd_model2 {
// A single verified original scene pistol is cosmetic for tracked play. HOTD
// has no verified native pickup/equipment selector; never borrow VC2 IDs.
inline constexpr int AutoOriginalGun=8;
inline constexpr std::array<int,3> SelectableGunChoices{AutoOriginalGun,0,1};
inline constexpr unsigned GunChoiceCount=SelectableGunChoices.size();
constexpr bool selectable_gun_choice(int c){for(int choice:SelectableGunChoices)if(c==choice)return true;return false;}
constexpr int cycle_gun_choice(int c,int d){
    for(unsigned i=0;i<GunChoiceCount;++i)if(SelectableGunChoices[i]==c)
        return SelectableGunChoices[(int(i)+d%int(GunChoiceCount)+int(GunChoiceCount))%int(GunChoiceCount)];
    return AutoOriginalGun;
}
struct WeaponChoice{unsigned manual_choice{},native_weapon{};bool follows_game{},standard_fallback{};};
constexpr WeaponChoice select_weapon(int choice,unsigned){
    if(choice==AutoOriginalGun)return {8,1,false,true};
    return {unsigned(choice==1?1:0),0,false,false};
}
inline WeaponChoice select_player_weapon(int choice,const GameState&,unsigned){return select_weapon(choice,0);}
}
