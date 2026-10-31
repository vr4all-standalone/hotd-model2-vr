// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cstdint>
namespace hotd_model2 {
struct GunCalibration {uint32_t xmin{},xmax{},ymin{},ymax{};bool valid()const{return xmin<xmax&&ymin<ymax&&xmax<=1023&&ymax<=1023;}};
struct PlayerState {
    uint32_t state{},ammo{},capacity{},shots{},weapon{};float health{};uint32_t native_state{};
    uint64_t feedback_epoch{};uint32_t feedback_shots{};
    bool joined()const{return state==1||state==2;}
    bool alive()const{return state==1&&health>0;}
};
struct GameState {
    bool valid{},playing{},counting_shots{};uint32_t mode{};
    std::array<PlayerState,2> players{};bool ready{},service{},gun_calibration{};
    uint32_t submode{},service_menu{},service_submode{},service_cursor{},service_busy{};
    std::array<GunCalibration,2> calibration{};
    uint32_t native_game_loops{},native_backlog_events{},native_vblank_pending{},native_video_control{};
    uint32_t start_cost{1},continue_cost{1};std::array<uint32_t,2> credits{};bool shared_credits{true},free_play{};
    std::array<uint16_t,2> native_coins{},coin_fraction{};
    std::array<uint32_t,2> credit_remainder{};
};
// Verified original hotdo program: main mode dispatcher 13a50, player init
// 13c2c/13c9c, accepted ammo decrement 5c00, service gate13a74 and native
// calibration EEPROM accessors1f0f0..1f120. Revision A is NOT assumed equal.
template<class Read32> GameState read_game_state(Read32 read){
    constexpr uint32_t code[][2]={{0x13a50,0xc8a03000},{0x13a54,0x0051ee14},
        {0x13a58,0x90a03914},{0x13a5c,0x000a99b0},
        {0x13c2c,0x92a83000},{0x13c30,0x0051ec04},
        {0x13c9c,0x92a83000},{0x13ca0,0x0051ecd0},
        {0x5c00,0x82ad1000},
        // Original committed-shot branch and refund path. The write at 5c3c
        // alone proceeds to hit processing/sound; 5c00 may be refunded.
        {0x5ab8,0x8c283914},{0x5abc,0x0051eef0},
        {0x5bbc,0x80a16019},{0x5bc0,0x3a052104},
        {0x5bd0,0x3a0520f4},{0x5c10,0x90a03000},{0x5c14,0x0051ef70},
        {0x5c18,0x3a052084},{0x5c1c,0x90a12008},{0x5c34,0x58a50981},
        {0x5c3c,0x92a12008},{0x5c40,0x0902c160},{0x5ca8,0x82e83000},
        {0x5cb0,0x3a052010},{0x5cbc,0x82a12041},
        {0x13a74,0x92a83000},{0x13a78,0x0051e258},
        {0x1f0f0,0x88803990},{0x1f0f4,0x0051f0b0},
        {0x1f110,0x88803990},{0x1f114,0x0051f0b4},
        {0xb470,0x5c801e00},{0xb4a0,0x5c801e01},
        {0x1b6c8,0xc8a83000},{0x1b6cc,0x0051e1a0},
        {0x1b6d8,0xc8803000},{0x1b6dc,0x0051e194},
        {0x1b6f4,0x8ab03000},{0x1b6f8,0x01d0000e}};
    for(const auto& w:code)if(uint32_t(read(w[0]))!=w[1])return {};
    const auto byte=[&](uint32_t a){return (uint32_t(read(a&~3u))>>((a&3)*8))&255u;};
    const auto half=[&](uint32_t a){return (uint32_t(read(a&~3u))>>((a&2)*8))&65535u;};
    GameState s;s.mode=half(0x51ee14);const auto service=read(0x51e258);
    if(s.mode>9||service>1)return {};
    s.service=service!=0;s.playing=!s.service&&s.mode==4;
    s.ready=!s.service&&s.mode>=1&&s.mode<=9;
    if(s.service){
        s.service_menu=read(0x512090);s.service_cursor=read(0x51209c);s.service_submode=read(0x512100);
        if(s.service_menu>11)return {};
        // The native Gun Setting list is a menu, not an aim phase. Actual
        // Gun Mark Checking/P1/P2 adjustment handlers alone own gun input.
        s.gun_calibration=s.service_menu==8&&
            ((s.service_cursor==0&&s.service_submode==0xb0c0)||
             (s.service_cursor==1&&s.service_submode==0xb470)||
             (s.service_cursor==4&&s.service_submode==0xb4a0));
        s.service_busy=s.gun_calibration?1:0;
    }
    s.start_cost=read(0x51ef84);s.continue_cost=read(0x51ef88);
    const auto free=read(0x51f370),split=read(0x51f374);
    if(s.start_cost>5||s.continue_cost>5||free>1||split>1)return {};
    s.free_play=free==1;s.shared_credits=split==0;
    for(unsigned i=0;i<2;++i){
        auto& p=s.players[i];const uint32_t base=0x51ec00+i*0xcc;
        if(byte(base)!=i)return {};
        p.native_state=read(base+4);p.ammo=byte(base+0x41);p.capacity=6;p.health=float(read(base+0x10));
        if(p.native_state>13||p.ammo>6||p.health>16)return {};
        // 5 is the native live gun state in the measured P1/P2 traces. 4 is
        // the continue state selected by native cost branch22bd4/6080.
        p.state=s.playing?(p.native_state==5?1:p.native_state==4?2:0):0;
        const uint32_t c=0x51f0b0+i*8;
        s.calibration[i]={half(c),half(c+4),half(c+2),half(c+6)};
        s.credits[i]=read(0x51ee38+i*4);if(s.credits[i]>999)return {};
        // Original accepted coin counters are 16-bit backup-SRAM bookkeeping,
        // updated once per actual slot event (P1 store1b6f4). Remainders expose
        // fractional credit progress; no one-coin/one-credit assumption.
        s.native_coins[i]=uint16_t(half(0x01d0000eu + i*2));
        s.credit_remainder[i]=read(0x51ee40+i*4);
        s.coin_fraction[i]=uint16_t(half(0x51e1ac+i*2));
    }
    s.native_game_loops=read(0x51eda8);
    // The engine's read-only committed-shot observer supplies cumulative totals.
    // A standalone RAM read never claims that an event observer is attached.
    s.valid=true;return s;
}
}
