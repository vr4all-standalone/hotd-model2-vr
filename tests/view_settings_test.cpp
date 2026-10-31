// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/settings_io.h"
#include "../native/controller_controls.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
using namespace hotd_model2;
static void require(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
static std::string saved(const std::string& text){std::istringstream input(text);std::ostringstream output;write_settings(output,read_settings(input));return output.str();}
static std::string current(const std::string& text){return saved("view_defaults_version=4\n"+text);}
static double value(const std::string& text,const std::string& key){
 const auto at=text.find(key+"=");require(at!=std::string::npos,"Saved view size is missing");return std::stod(text.substr(at+key.size()+1));
}
int main(){
 require(Settings{}.world_scale==0.10f&&Settings{}.view_size==1,"Fresh settings do not use world scale 0.10 and view size 100%");
 const auto legacy=saved("version=7\nview_defaults_version=3\nworld_scale=0.102\nview_size=.75\ngun_angle_zero_version=1\ngun_angle=17\ngun=1\np1_hand=1\n");
 require(std::abs(value(legacy,"world_scale")-0.10)<1e-6&&value(legacy,"view_size")==1,"Old 0.115/zoomed settings did not adopt the requested defaults");
 require(value(legacy,"gun")==1&&value(legacy,"p1_hand")==1&&value(legacy,"gun_angle")==17,"View default migration changed unrelated choices");
 require(std::abs(value(current("version=7\nworld_scale=0.115\nview_size=.75\n"),"world_scale")-0.115)<1e-6,"Later world scale preferences were reset again");
 const auto file=std::filesystem::temp_directory_path()/("hotd-view-defaults-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".ini");
 {std::ofstream out(file);out<<"version=7\nview_defaults_version=3\ngun_angle_zero_version=1\ngun_angle=17\nworld_scale=0.102\nview_size=.75\ncalibration_native=keep\nunknown_future=preserve\n";}
 const auto first=load_settings(file.string().c_str());require(first.world_scale==0.10f&&first.view_size==1&&first.gun_angle==17,"Actual load did not migrate only the requested view defaults");
 std::ifstream persisted(file);std::string text{std::istreambuf_iterator<char>(persisted),{}};persisted.close();
 require(text.find("view_defaults_version=4\n")!=std::string::npos&&text.find("calibration_native=keep\n")!=std::string::npos&&text.find("unknown_future=preserve\n")!=std::string::npos,"Migration did not persist its marker or preserve unknown settings");
 {auto at=text.find("world_scale=0.1\n");require(at!=std::string::npos,"World default was not written");text.replace(at,std::string("world_scale=0.1\n").size(),"world_scale=0.115\n");std::ofstream out(file);out<<text;}
 require(load_settings(file.string().c_str()).world_scale==0.115f,"Second actual load overwrote a later explicit world scale");std::filesystem::remove(file);
 // A valid new preference must survive a real legacy-settings load/write cycle.
 auto migrated=current("version=6\nview_size=0.75\nworld_scale=0.02\npanel_distance=2.4\ngun_scale=1.3\np1_hand=1\ngun_angle_zero_version=1\ngun_angle=17\n");
 require(std::abs(value(migrated,"view_size")-.75)<1e-6,"View size did not survive settings load/save");
 require(std::abs(value(migrated,"world_scale")-.02)<1e-6&&std::abs(value(migrated,"panel_distance")-2.4)<1e-6&&std::abs(value(migrated,"gun_scale")-1.3)<1e-6,"View size changed independent world/panel/gun preferences");
 require(value(migrated,"p1_hand")==1&&value(migrated,"gun_angle")==17,"View migration changed hand ownership or gun angle");
 require(std::abs(value(saved(migrated),"view_size")-.75)<1e-6,"View size did not persist on the next launch");
 for(int version=1;version<=6;version++)require(value(saved("version="+std::to_string(version)+"\n"),"view_size")==1,"Legacy view framing changed without an explicit preference");
 for(const char* invalid:{"nan","inf","0","0.49","1.01","2","bad",".75 trailing"})require(value(saved(std::string("version=6\nview_size=")+invalid+"\n"),"view_size")==1,"Invalid view size changed the default framing");
 require(value(current("version=6\nview_size=.5\n"),"view_size")==.5&&value(current("version=6\nview_size=1\n"),"view_size")==1,"View size endpoints did not survive persistence");
 require(value(saved(""),"gun")==8,"New settings did not offer the verified original pistol by default");
 for(int version=3;version<=7;version++)for(int gun:{0,1,8})require(value(saved("version="+std::to_string(version)+"\ngun="+std::to_string(gun)+"\n"),"gun")==gun,"Settings migration replaced an explicit gun preference");
 std::cout<<"View defaults 0.10/100%, one-time migration, persistence and unrelated preference preservation passed\n";
}
