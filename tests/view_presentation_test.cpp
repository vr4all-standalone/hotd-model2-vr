// SPDX-License-Identifier: GPL-2.0-or-later
#include "presentation.h"
#include "settings_io.h"
#include <fstream>
#include "gl_context.h"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace hotd_model2;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main()try {
 const auto file=std::filesystem::temp_directory_path()/("hotd-view-menu-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".ini");
 struct Cleanup {std::filesystem::path path;~Cleanup(){std::error_code ec;std::filesystem::remove(path,ec);}} cleanup{file};
 _putenv_s("HOTD_MODEL2_SETTINGS",file.string().c_str());
 GLContext context;Settings settings;settings.world_scale=.01f;SharedRuntime runtime;Presentation ui;PresentationInput input;double now=1;
 auto update=[&](){ui.update(input,settings,runtime,now);now+=.2;};
 auto move=[&](float x,float y){input.sticks[0]={x,y};update();input.sticks[0]={};update();};
 auto down=[&](int count){for(int i=0;i<count;i++)move(0,-1);};
 input.hands[0].tracked=true;input.poses[0][3].x=10;
 auto press=[&](){input.hands[0].start=true;update();input.hands[0].start=false;update();};
 auto tab=[&](int right){move(0,1);for(int i=0;i<right;i++)move(1,0);down(1);};
 update();input.toggle=true;update();input.toggle=false;update();require(ui.open(),"Menu did not open");
 require(settings.gun==8,"New menu did not start with the verified Original option");
 move(1,0);require(settings.gun==0,"Original did not cycle to Arcade");move(1,0);require(settings.gun==1,"Arcade did not cycle to Zapper");move(1,0);require(settings.gun==8,"Zapper did not cycle back to Original");
 tab(1);down(1);press();move(1,0);require(settings.world_scale==.02f,"World menu must use0.01 steps at its minimum");
 move(-1,0);require(settings.world_scale==.01f,"World minimum reverse step failed");
 move(-1,0);require(settings.world_scale==.01f,"World minimum must clamp");
 settings.world_scale=.10f;
 for(int i=11;i<=200;i++){move(1,0);require(std::abs(settings.world_scale-i/100.f)<1e-6,"World menu skipped a0.01 forward step or offered removed fraction");}
 move(1,0);require(settings.world_scale==2.f,"World maximum must clamp");
 for(int i=199;i>=1;i--){move(-1,0);require(std::abs(settings.world_scale-i/100.f)<1e-6,"World menu skipped a0.01 reverse step");}
 for(float old:{.102f,.105f,.114617f}){settings.world_scale=old;move(-1,0);require(settings.world_scale==(old<.11f?.10f:.11f),"Off-grid saved value must step down onto0.01 grid");}
 settings.world_scale=.102f;move(1,0);require(settings.world_scale==.11f,"Removed0.102 must step up to0.11");
 settings.world_scale=.10f;move(1,0);require(settings.world_scale==.11f,"0.10 must go directly to0.11");
 move(1,0);require(settings.world_scale==.12f,"0.11 must go directly to0.12");
 down(1);move(1,0);require(std::abs(settings.panel_distance-1.9f)<1e-6&&settings.view_size==1,"Panel distance changed view size");
 down(1);move(1,0);require(std::abs(settings.hud_size-1.05f)<1e-6&&settings.panel_distance==1.9f,"HUD size must change in 5% steps");
 for(int i=0;i<30;i++)move(1,0);require(settings.hud_size==2,"HUD maximum clamp");for(int i=0;i<40;i++)move(-1,0);require(settings.hud_size==.5f,"HUD minimum clamp");
 {std::ifstream in(file);require(read_settings(in).hud_size==.5f,"HUD menu save/load");}
 for(int i=0;i<3;i++)move(0,1);press();
 move(-1,0);require(std::abs(settings.view_size-.95f)<1e-6&&std::abs(settings.panel_distance-1.9f)<1e-6,"View size changed panel distance");
 for(int i=0;i<12;i++)move(-1,0);require(settings.view_size==.5f,"View size minimum clamp");
 tab(1);down(1);press();require(runtime.service_requested&&!ui.open(),"System Service must send native request");
 input.toggle=true;update();input.toggle=false;update();tab(2);down(4);press();require(!runtime.quit_requested,"Quit did not require confirmation");down(1);press();require(runtime.quit_requested,"Quit confirmation failed");
 {
  Presentation menu;Settings options;options.hud_size=2;options.reticle_max_size=2;options.p1_hand=1;SharedRuntime bridge;PresentationInput buttons;double tick=1;
  buttons.hands[0].tracked=true;buttons.poses[0][3].x=10;
  auto update_reset=[&]{menu.update(buttons,options,bridge,tick);tick+=.2;};
  auto step_reset=[&](float x,float y){buttons.sticks[0]={x,y};update_reset();buttons.sticks[0]={};update_reset();};
  auto press_reset=[&]{buttons.hands[0].start=true;update_reset();buttons.hands[0].start=false;update_reset();};
  update_reset();buttons.toggle=true;update_reset();buttons.toggle=false;update_reset();
  step_reset(0,1);step_reset(1,0);step_reset(1,0);step_reset(0,-1);step_reset(-1,0);
  require(options.p1_hand==0&&options.hud_size==2,"Hand transition changed saved HUD");
  for(int i=0;i<3;i++)step_reset(0,-1);
  press_reset();require(options.hud_size==2,"Reset must require confirmation");step_reset(0,-1);press_reset();
  require(options.reticle_max_size==.60f&&options.hud_size==1&&menu.open()&&!bridge.quit_requested,"Real Reset must restore HUD100 while staying in menu");
  std::ifstream saved(file);auto reset=read_settings(saved);require(reset.reticle_max_size==.60f&&reset.hud_size==1,"Real Reset must persist HUD100");
 }
 std::cout<<"Real menu view/world/panel/gun controls, supported row order, service and quit passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
