// SPDX-License-Identifier: GPL-2.0-or-later
#ifdef NDEBUG
#error "Native flash checks require active assertions"
#endif
#include "../native/native_flash.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#ifdef HOTD_FLASH_NATIVE_MASK_HARNESS
#include HOTD_FLASH_NATIVE_MASK_HARNESS
#endif
using namespace hotd_model2;
#define CHECK(condition) do {if(!(condition)){std::cerr<<"Native flash check failed at line "<<__LINE__<<": " #condition "\n";std::exit(1);}}while(false)
namespace {
std::vector<uint8_t> bytes(const std::filesystem::path& path) {
    std::ifstream in(path,std::ios::binary);CHECK(in);
    return {std::istreambuf_iterator<char>(in),{}};
}
uint32_t word(const std::vector<uint8_t>& data,size_t at) {
    CHECK(at+4<=data.size());uint32_t value;std::memcpy(&value,data.data()+at,4);return value;
}
void store(std::vector<uint8_t>& data,size_t at,uint32_t value) {
    CHECK(at+4<=data.size());std::memcpy(data.data()+at,&value,4);
}
struct Memory {
    std::vector<uint8_t> program,ram;
    // The old trace did not retain tile RAM. Reconstruct only the visible mask
    // written by original-ROM 18600 (192 stq writes); never label it captured.
    std::array<uint32_t,0xc00/4> mask{};
    uint32_t read(uint32_t at) const {
        if(at<program.size())return word(program,at);
        if(at>=0x500000&&at<0x600000)return word(ram,at-0x500000);
        if(at>=0x100c000&&at<0x100cc00)return mask.at((at-0x100c000)/4);
        CHECK(false&&"Unexpected flash-reader address");return 0;
    }
    void set(uint32_t at,uint32_t value){store(ram,at-0x500000,value);}
};
}
int main(int argc,char** argv) {
    if(argc!=3){std::cerr<<"Need private original program.bin and actual-scene-reference-v1/scenes\n";return 2;}
    const auto scenes=std::filesystem::path(argv[2]);
    Memory flash{bytes(argv[1]),bytes(scenes/"frame-00002015/workram.bin")};
    CHECK(flash.program.size()==0x200000&&flash.ram.size()==0x100000);
    flash.mask.fill(0xffffffffu);
    auto read=[&](uint32_t at){return flash.read(at);};
    const auto original=flash;
    // Removing the production detector must fail here on real original-ROM/RAM.
    CHECK(native_gun_strobe(read));
    CHECK(flash.program==original.program&&flash.ram==original.ram&&flash.mask==original.mask);
    const auto raster=bytes(scenes/"frame-00002015/foreground-argb32.bin");
    CHECK(raster.size()==496*384*4);
    for(size_t at=0;at<raster.size();at+=4)CHECK(word(raster,at)==0xffe4e4e4u);
    Memory fade=flash;fade.ram=bytes(scenes/"frame-00001151/workram.bin");
    CHECK(!native_gun_strobe([&](uint32_t at){return fade.read(at);}));
    Memory ordinary=flash;ordinary.ram=bytes(scenes/"frame-00002303/workram.bin");
    CHECK(!native_gun_strobe([&](uint32_t at){return ordinary.read(at);}));
    // Isolate each native gate while retaining the real ROM and positive RAM.
    for(auto mode:{0u,1u,3u,8u,9u}){flash=original;flash.set(0x51ee14,mode);CHECK(!native_gun_strobe(read));}
    for(auto cooldown:{0u,1u,2u,3u,5u,255u}){flash=original;flash.set(0x51ef6c,cooldown);CHECK(!native_gun_strobe(read));}
    flash=original;flash.set(0x51e258,1);CHECK(!native_gun_strobe(read));
    flash=original;flash.set(0x51ee34,4);CHECK(!native_gun_strobe(read));
    flash=original;flash.set(0x51eee4,0x23);CHECK(!native_gun_strobe(read));
    flash=original;flash.set(0x5430b0,0);CHECK(!native_gun_strobe(read));
    flash=original;flash.set(0x5430b0,2);CHECK(!native_gun_strobe(read));
    for(auto state:{0u,4u,9u,13u,14u,0xffffffffu}){flash=original;flash.set(0x51ec04,state);CHECK(!native_gun_strobe(read));}
    for(auto state:{5u,8u,11u}){flash=original;flash.set(0x51ec04,state);CHECK(native_gun_strobe(read));}
    // P2 requests and simultaneous reads do not depend on ammo/accepted shots.
    flash=original;flash.set(0x5430b0,0x100);flash.set(0x51ecd0,5);CHECK(native_gun_strobe(read));
    flash.set(0x5430b0,0x101);flash.set(0x51ec40,0);flash.set(0x51ed0c,0);CHECK(native_gun_strobe(read));
    for(size_t at:{size_t(0),size_t(0x7fc/4),size_t(0xbfc/4)}){
        flash=original;flash.mask[at]^=1;CHECK(!native_gun_strobe(read));
    }
    flash=original;flash.program[0x45ea0]^=1;CHECK(!native_gun_strobe(read));
    flash=original;flash.program[0x1861c]^=1;CHECK(!native_gun_strobe(read));
    flash=original;flash.program[0xa8786+5*8]=0;CHECK(!native_gun_strobe(read));
    unsigned reads=0;
    CHECK(!native_gun_strobe([&](uint32_t){++reads;return 0xffffffffu;}));CHECK(reads==1);
    flash=original;
    const auto* override_mask=native_gun_window_mask(true,"hotdo",false,read);CHECK(override_mask);
    for(size_t at=0;at<0x800;++at)CHECK(override_mask[at]==0);
#ifdef HOTD_FLASH_NATIVE_MASK_HARNESS
    // Execute the engine's actual tile-mask draw functions, extracted without
    // edits by test_native_flash.py. Bitmap/device storage alone is a fixture.
    segas24_tile_device tiles;screen_device screen;rectangle clip{0,495,0,383};
    bitmap_rgb32 output(496,384);
    tiles.colors.values[1]=word(raster,0);tiles.colors.values[2]=0xff123456;
    std::fill_n(tiles.tile_ram.get()+0x6000,0x600,uint16_t(0xffff));
    std::fill(tiles.tile_layer[1]->pixels.storage.begin(),tiles.tile_layer[1]->pixels.storage.end(),uint16_t(1));
    std::fill(tiles.tile_layer[1]->flags.storage.begin(),tiles.tile_layer[1]->flags.storage.end(),uint8_t(TILEMAP_PIXEL_LAYER0|1));
    tiles.tile_layer[0]->pixels.pix(8,8)=2;tiles.tile_layer[0]->flags.pix(8,8)=TILEMAP_PIXEL_LAYER0|1;
    auto foreground=[&](const uint16_t* mask){
        std::fill(output.storage.begin(),output.storage.end(),0);
        for(int layer=3;layer>=0;--layer)tiles.draw_with_window_mask(screen,output,clip,(layer<<1)|1,0,0,mask);
    };
    const auto native_tile_ram=std::vector<uint16_t>(tiles.tile_ram.get(),tiles.tile_ram.get()+0x8000);
    foreground(nullptr);for(auto pixel:output.storage)CHECK(pixel==0xffe4e4e4u);
    foreground(override_mask);CHECK(output.pix(0,0)==0&&output.pix(8,8)==0xff123456u);
    // The secondary tile pair keeps its native mask even during suppression.
    tiles.tile_layer[2]->pixels.pix(16,16)=2;tiles.tile_layer[2]->flags.pix(16,16)=TILEMAP_PIXEL_LAYER0|1;
    foreground(override_mask);CHECK(output.pix(16,16)==0xff123456u);
    foreground(nullptr);for(auto pixel:output.storage)CHECK(pixel==0xffe4e4e4u);
    CHECK(std::equal(native_tile_ram.begin(),native_tile_ram.end(),tiles.tile_ram.get()));
    std::cout<<"Actual native tile renderer removes strobe selection, preserves HUD/secondary pair, restores next draw and never changes tile RAM\n";
#endif
    auto forbidden=[](uint32_t)->uint32_t{CHECK(false&&"Out-of-scope render must not read emulated memory");return 0;};
    CHECK(!native_gun_window_mask(false,"hotdo",false,forbidden));
    CHECK(!native_gun_window_mask(true,"hotd",false,forbidden));
    CHECK(!native_gun_window_mask(true,"hotdo",true,forbidden));
    flash=ordinary;CHECK(!native_gun_window_mask(true,"hotdo",false,read));
    flash=fade;CHECK(!native_gun_window_mask(true,"hotdo",false,read));
    flash=original;CHECK(native_gun_window_mask(true,"hotdo",false,read));
    CHECK(flash.program==original.program&&flash.ram==original.ram&&flash.mask==original.mask);
    std::cout<<"Original frame2015 strobe identified; frame1151 white fade, ordinary gameplay, service, unknown ROM and partial masks retained; native data unchanged\n";
}
