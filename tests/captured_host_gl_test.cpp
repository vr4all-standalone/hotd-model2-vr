// SPDX-License-Identifier: GPL-2.0-or-later
// Exercise the real host DLL simulation entry point with an owned game frame.
#include "captured_frame_io.h"
#include "shared_runtime.h"
#include <windows.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using namespace hotd_model2;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void environment(const char* name,const char* value){_putenv_s(name,value?value:"");SetEnvironmentVariableA(name,value);}
void await_image(const std::filesystem::path& file,const SharedRuntime& shared) {
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(std::chrono::steady_clock::now()<deadline) {
        if(shared.quit_requested)throw std::runtime_error("Production host failed; inspect host.log");
        if(std::filesystem::exists(file)&&std::filesystem::file_size(file)>100)return;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    throw std::runtime_error("Production host did not write captured-game image");
}
}
int main(int argc,char** argv)try {
    if(argc!=5){std::cerr<<"usage: captured_host_gl_test geometry.txt capture-dir host.dll output-dir\n";return 2;}
    auto frame=test::read_frame(argv[1],argv[2]);const std::filesystem::path output=argv[4];
    std::filesystem::create_directories(output);
    environment("HOTD_MODEL2_XR_SIM","1");
    environment("HOTD_MODEL2_SIM_DIR",output.string().c_str());
    environment("HOTD_MODEL2_XR_LOG",(output/"host.log").string().c_str());
    for(const auto* env:{"HOTD_MODEL2_SIM_GUN_TEST","HOTD_MODEL2_SIM_SERVICE_TEST","HOTD_MODEL2_SIM_REFERENCE","HOTD_MODEL2_SIM_WIDER_VIEW","HOTD_MODEL2_SIM_OFFSET_X"})environment(env,nullptr);
    require(std::getenv("HOTD_MODEL2_XR_SIM")!=nullptr,"Diagnostic simulation environment missing; refuse runtime start");
    auto library=LoadLibraryA(argv[3]);require(library,"Cannot load actual production host DLL");
    auto start=reinterpret_cast<bool(*)(SharedRuntime*)>(GetProcAddress(library,"hotd_model2_xr_start"));
    auto stop=reinterpret_cast<void(*)()>(GetProcAddress(library,"hotd_model2_xr_stop"));
    require(start&&stop,"Actual production host exports missing");
    SharedRuntime shared;
    // A native service/raster panel writes a nearer depth sheet. The next
    // gameplay frame must not inherit that previous frame's depth buffer.
    auto panel=std::make_shared<NativeFrame>(*frame);panel->sequence=120;panel->raster_only=true;
    panel->native.assign(size_t(frame->width)*frame->height,0xff000000u);
    shared.scene.store(panel);require(start(&shared),"Actual production host rejected start");
    try {
        await_image(output/"live-120.ppm",shared);
        shared.scene.store(frame);
        await_image(output/("live-"+std::to_string(frame->sequence)+".ppm"),shared);
    }catch(...){stop();FreeLibrary(library);throw;}
    stop();FreeLibrary(library);
    std::ifstream in(output/("live-"+std::to_string(frame->sequence)+".ppm"),std::ios::binary);
    std::string magic;unsigned width{},height{},maximum{};in>>magic>>width>>height>>maximum;in.get();
    require(magic=="P6"&&width==unsigned(frame->width)&&height==unsigned(frame->height)&&maximum==255,"Invalid production host image");
    std::vector<uint8_t> pixels{std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
    require(pixels.size()==size_t(width)*height*3,"Incomplete production host image");
    size_t world{};
    for(size_t i=0;i<frame->foreground.size();i++)if(frame->foreground[i]==0&&(pixels[i*3]||pixels[i*3+1]||pixels[i*3+2]))++world;
    std::cout<<"Actual captured HOTD host world pixels after nearer prior frame: "<<world<<'\n';
    require(world>5000,"Actual captured HOTD world is black/occluded after prior raster frame");
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
