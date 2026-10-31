// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/game_state.h"
#include "../native/camera_state.h"
#include "../native/native_weapon.h"
#include "../native/native_flash.h"
#include <cassert>
#include <iostream>
using namespace hotd_model2;
int main() {
    unsigned reads=0;
    auto hostile=[&](uint32_t){++reads;return 0xffffffffu;};
    const auto state=read_game_state(hostile);
    assert(!state.valid && !state.ready && !state.playing && !state.counting_shots);
    for(auto p:state.players)assert(!p.joined() && !p.alive());
    assert(reads==1); // Mismatch stops at the first HOTD ROM fingerprint; no RAM read.
    assert(!camera_revision_matches(hostile));
    assert(camera_write(0x506044,0x12345678,~0u,hostile,110)==0x12345678);
    assert(!native_gun_strobe(hostile));
    assert(reads==2); // Strobe rejects its first original-program fingerprint.
    assert(!extract_native_weapon(hostile,hostile,hostile,1,0x800000,0x800000).valid);
    assert(reads==3); // Original pistol rejects before reading model/texture data.
    std::cout<<"Unverified HOTD program state fails closed without memory access\n";
}
