// SPDX-License-Identifier: GPL-2.0-or-later
#ifdef NDEBUG
#error "Native fixture checks require active assertions"
#endif
#include "../native/game_state.h"
#include <cassert>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <iostream>
using namespace hotd_model2;
std::vector<unsigned char> bytes(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);assert(f);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){
    if(argc!=3&&argc!=4){std::cerr<<"Need private program.bin, native-trace-v1, optional calibration-native-v1\n";return 2;}
    auto program=bytes(argv[1]);assert(program.size()==0x200000);
    const auto backup=bytes(std::filesystem::path(argv[2])/"nvram/hotdo/backup1");assert(backup.size()==0x4000);
    for(unsigned sec:{5,15,20,35,40,55,65}) {
        char file[32];std::snprintf(file,sizeof file,"ram-%03u.bin",sec);auto ram=bytes(std::filesystem::path(argv[2])/file);
        auto read=[&](uint32_t a){uint32_t v;const unsigned char* p=a<program.size()?program.data()+a:a>=0x500000&&a+4<=0x600000?ram.data()+a-0x500000:a>=0x1d00000&&a+4<=0x1d04000?backup.data()+a-0x1d00000:nullptr;assert(p);std::memcpy(&v,p,4);return v;};
        auto s=read_game_state(read);assert(s.valid);
        if(sec>=15)assert(s.start_cost==2&&s.continue_cost==1);
        if(sec==5)assert(!s.ready&&!s.playing);
        else if(sec==15)assert(s.ready&&!s.playing&&!s.players[0].joined()&&!s.players[1].joined());
        else if(sec==20||sec==35)assert(s.ready&&s.playing&&s.players[0].alive()&&!s.players[1].joined()&&s.players[0].ammo==6);
        else if(sec==40)assert(s.players[0].alive()&&s.players[1].alive()&&s.players[0].ammo==1&&s.players[1].ammo==6);
        else if(sec==55)assert(s.players[0].ammo==0&&s.players[1].ammo==0);
        else if(sec==65)assert(s.service&&!s.ready&&!s.playing);
        assert(s.calibration[0].valid()&&s.calibration[1].valid());
        assert(s.calibration[0].xmin==173&&s.calibration[0].xmax==596&&s.calibration[0].ymin==87&&s.calibration[0].ymax==380);
        if(sec==40){
            const auto old=ram[0x1ef84];ram[0x1ef84]=6;assert(!read_game_state(read).valid);ram[0x1ef84]=old;
            ram[0x1ec00]=1;assert(!read_game_state(read).valid);ram[0x1ec00]=0;
        }
    }
    if(argc==4)for(unsigned sec:{29,38,41,46}){
        char file[32];std::snprintf(file,sizeof file,"ram-%03u.bin",sec);auto ram=bytes(std::filesystem::path(argv[3])/file);
        auto read=[&](uint32_t a){uint32_t v;const unsigned char* p=a<program.size()?program.data()+a:a>=0x500000&&a+4<=0x600000?ram.data()+a-0x500000:a>=0x1d00000&&a+4<=0x1d04000?backup.data()+a-0x1d00000:nullptr;assert(p);std::memcpy(&v,p,4);return v;};
        const auto s=read_game_state(read);assert(s.valid&&s.service&&s.service_menu==8);
        assert(s.gun_calibration==(sec==38||sec==46));
        assert(s.service_cursor==(sec==29?7:sec==46?1:0));
    }
    program[0x13a50]^=1;
    auto bad=[&](uint32_t a){assert(a+4<=program.size());uint32_t v;std::memcpy(&v,program.data()+a,4);return v;};
    assert(!read_game_state(bad).valid);
    std::cout<<"Actual HOTD original-revision boot, native P1/P2, ammo, service and calibration snapshots passed\n";
}
