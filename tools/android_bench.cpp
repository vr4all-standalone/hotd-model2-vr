// SPDX-License-Identifier: GPL-2.0-or-later
// Adapted from the existing VR4ALL Model2 diagnostic; no OpenXR session.
#include <dlfcn.h>
#include <cstdlib>
#include <cstdio>
extern "C" const char* SDL_AndroidGetExternalStoragePath(){return "/data/local/tmp/hotd-profile";}
extern "C" const char* SDL_AndroidGetInternalStoragePath(){return "/data/local/tmp/hotd-profile";}
extern "C" void* SDL_AndroidGetJNIEnv(){return nullptr;}
extern "C" void* SDL_AndroidGetActivity(){return nullptr;}
int main(int argc,char** argv){
    setenv("SDL_VIDEODRIVER","dummy",1);setenv("SDL_AUDIODRIVER","dummy",1);
    if(!std::getenv("HOTD_MODEL2_BENCH_CAPTURE"))unsetenv("HOTD_MODEL2_VR");
    const char* name=std::getenv("HOTD_MODEL2_BENCH_LIBRARY");
    void* lib=dlopen(name?name:"./libmain.so",RTLD_NOW|RTLD_GLOBAL);
    if(!lib){std::fprintf(stderr,"Cannot load benchmark engine: %s\n",dlerror());return 2;}
    auto ready=reinterpret_cast<void(*)()>(dlsym(lib,"SDL_SetMainReady"));
    auto run=reinterpret_cast<int(*)(int,char**)>(dlsym(lib,"SDL_main"));
    if(!ready||!run){std::fprintf(stderr,"Missing SDL entry points\n");return 3;}
    ready();return run(argc,argv);
}
