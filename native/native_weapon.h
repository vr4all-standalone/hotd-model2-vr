// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <vector>
namespace hotd_model2 {
struct NativeWeaponVertex{float x{},y{},z{},u{},v{};};
struct NativeWeaponPolygon{unsigned count{};uint32_t attr{};std::array<uint16_t,4> header{};std::array<float,3> normal{};std::array<NativeWeaponVertex,4> vertices{};};
struct NativeWeapon{bool valid{};uint32_t weapon{},model{},object{};std::vector<NativeWeaponPolygon> polygons;};
// Original hotdo's named PN_ter00a is a scene pistol with its authored right
// hand. It is a cosmetic tracked weapon: a continuously rendered first-person
// gun or a VC2-style equipment/pickup state has not been verified. See
// docs/native-weapon-evidence.md. Coordinates here retain native source units.
inline const char* native_weapon_name(unsigned weapon){return weapon==1?"Original HOTD scene pistol":"Unavailable";}
template<class ReadProgram,class ReadPolygon,class ReadTexture>
NativeWeapon extract_native_weapon(ReadProgram read_program,ReadPolygon read_polygon,ReadTexture read_texture,
                                  unsigned weapon,uint32_t polygon_words,uint32_t texture_words) {
    if(weapon!=1||!polygon_words||!texture_words)return {};
    // Original model draw13fcc copies a16-byte HOTD record directly to TGP.
    // HOTD records are tp,th,object,count (a different order from VC1/VC2).
    constexpr uint32_t fingerprint[][2]={
        {0x13fcc,0x8c803a10},{0x13fd0,0x02e73530},{0x13fd4,0xb0a41000},
        {0x13fd8,0xb2a03000},{0x13fdc,0x00804000},
        {0xb47f0,0x719},{0xb4aa0,0x719},{0xbbf20,0x719},{0xbd050,0x719},
        {0x02e54560,0x745f4e50},{0x02e54564,0x30307265},{0x02e54568,0x61}};
    for(const auto& item:fingerprint)if(uint32_t(read_program(item[0]))!=item[1])return {};
    constexpr uint32_t entry=0x02e73530+0x719*16;
    uint32_t tp=read_program(entry),th=read_program(entry+4);
    const uint32_t object=read_program(entry+8),count=read_program(entry+12);
    if(tp!=0x9764e||th!=0x977de||object!=0x8e7f31||count!=51)return {};
    uint32_t at=object&0x7fffff;
    if(uint64_t(at)+6>polygon_words)return {};
    const auto point=[&](uint32_t index){return std::array<float,3>{
        std::bit_cast<float>(uint32_t(read_polygon(index))),
        std::bit_cast<float>(uint32_t(read_polygon(index+1))),
        std::bit_cast<float>(uint32_t(read_polygon(index+2)))};};
    const auto finite=[](const auto& p){for(float value:p)if(!std::isfinite(value)||std::abs(value)>1000000.f)return false;return true;};
    auto p0=point(at),p1=point(at+3);at+=6;
    if(!finite(p0)||!finite(p1))return {};
    NativeWeapon result;result.weapon=1;result.model=0x719;result.object=object;
    for(uint32_t link=0;link<=count;++link) {
        if(at>=polygon_words)return {};
        const uint32_t attr=read_polygon(at++);
        if(!(attr&3)) {
            if(link!=count||result.polygons.size()!=50)return {};
            result.valid=true;return result;
        }
        if(uint64_t(at)+9>polygon_words)return {};
        const auto normal=point(at);auto p2=point(at+3),p3=point(at+6);at+=9;
        const unsigned n=(attr&1)?4:3;if(n==3)p3=p2;
        if(!finite(normal)||!finite(p2)||!finite(p3)||uint64_t(tp)+n*2>texture_words||uint64_t(th)+4>texture_words)return {};
        NativeWeaponPolygon polygon;polygon.count=n;polygon.attr=attr;polygon.normal=normal;
        const std::array<std::array<float,3>,4> points={p1,p0,p2,p3};
        for(unsigned i=0;i<4;++i)polygon.header[i]=uint16_t(read_texture(th+i));
        for(unsigned i=0;i<n;++i) {
            const auto& p=points[i];polygon.vertices[i]={p[0],p[1],p[2],float(read_texture(tp+i*2+1)),float(read_texture(tp+i*2))};
        }
        tp+=n*2;int offset=int((attr>>12)&31);if(offset&16)offset-=32;
        const int64_t next=int64_t(th)+offset*4;
        if(next<0||uint64_t(next)>=texture_words)return {};
        th=uint32_t(next);
        const unsigned kind=(attr>>8)&3;
        if(kind)result.polygons.push_back(polygon);
        if(kind==0||kind==2){p0=p2;p1=p3;}else if(kind==1)p1=p2;else p0=p3;
    }
    return {};
}
}
