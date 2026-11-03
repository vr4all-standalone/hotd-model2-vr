// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "input_policy.h"
#include "weapon_choice.h"
#include <istream>
#include <fstream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
namespace hotd_model2 {
inline Settings read_settings(std::istream& input,bool* angle_migrated=nullptr,bool* view_migrated=nullptr) {
    bool angle_zero_version=false,view_defaults_version=false,has_reticle_distance=false;
    Settings out;std::string line;std::vector<std::string> lines;bool immersive_version=false,original_gun_version=false,three_gun_version=false;
    while(std::getline(input,line)){if(line=="version=2"||line=="version=3"||line=="version=4"||line=="version=5"||line=="version=6"||line=="version=7")immersive_version=true;if(line=="version=3"||line=="version=4"||line=="version=5"||line=="version=6"||line=="version=7")original_gun_version=true;if(line=="version=4"||line=="version=5"||line=="version=6"||line=="version=7")three_gun_version=true;lines.push_back(line);}
    for(const auto& line:lines) {
        auto equals=line.find('=');if(equals==std::string::npos)continue;
        auto key=line.substr(0,equals);std::istringstream value(line.substr(equals+1));double number{};char extra{};
        if(!(value>>number)||(value>>extra)||!std::isfinite(number))continue;
        auto integer=[&](int lo,int hi){return number==std::floor(number)&&number>=lo&&number<=hi;};
        if(key=="gun_angle_zero_version"&&integer(1,2147483647))angle_zero_version=true;
        else if(key=="view_defaults_version"&&integer(4,2147483647))view_defaults_version=true;
        else if(key=="p1_hand"&&integer(0,1))out.p1_hand=int(number);
        else if(key=="reload"&&integer(0,2))out.reload=number==2?ReloadMode::PointOutside:ReloadMode::ShootOutside;
        else if(key=="gun"&&integer(0,8)) {
            const int choice=int(number);
            if(three_gun_version) {if(selectable_gun_choice(choice))out.gun=choice;}
            else out.gun=choice==1?1:choice==0&&original_gun_version?0:AutoOriginalGun;
        }
        else if(key=="laser"&&integer(0,1))out.laser=number!=0;
        else if(key=="laser_width"&&integer(1,3))out.laser_width=number;
        else if(key=="reticle_mode"&&integer(1,2))out.reticle_mode=reticle_mode_value(int(number));
        else if(key=="reticle_min_size"&&number>=.05&&number<=1)out.reticle_min_size=float(number);
        else if(key=="reticle_max_size"&&number>=.25&&number<=3)out.reticle_max_size=float(number);
        else if(key=="reticle_shrink_distance"&&number>=.2&&number<=20){out.reticle_shrink_distance=float(number);has_reticle_distance=true;}
        // Game zoom is session-only: ignore legacy saved On values so every
        // new VR launch starts in the accepted unzoomed presentation.
        // Wider view is unavailable pending HOTD-specific camera evidence.
        else if(key=="background"&&integer(0,1))out.background=BackgroundMode(int(number));
        else if(key=="view_fov"&&immersive_version&&number>=80&&number<=140)out.view_fov=float(number);
        else if(key=="world_scale"&&immersive_version&&number>=.01&&number<=2)out.world_scale=float(number);
        else if(key=="view_size"&&number>=.5&&number<=1)out.view_size=float(number);
        else if(key=="hud_size"&&number>=.5&&number<=2)out.hud_size=float(number);
        else if(key=="panel_distance"&&number>=.5&&number<=10)out.panel_distance=float(number);
        else if(key=="gun_angle"&&number>=-30&&number<=30)out.gun_angle=float(number);
        else if(key=="gun_scale"&&number>=.4&&number<=1.6)out.gun_scale=float(number);
        else if(key=="haptics"&&number>=0&&number<=1)out.haptics=HapticLevels[haptic_level(float(number))];
    }
    // Use the current default for missing/invalid values; preserve explicit choices.
    if(!has_reticle_distance)out.reticle_shrink_distance=Settings{}.reticle_shrink_distance;
    if(!angle_zero_version)out.gun_angle=0;
    // Apply the user's accepted world/framing baseline once, then preserve
    // subsequent manual choices independently of the legacy file version.
    if(!view_defaults_version){out.world_scale=0.1f;out.view_size=1.f;}
    if(angle_migrated)*angle_migrated=!angle_zero_version;
    if(view_migrated)*view_migrated=!view_defaults_version;
    return out;
}
inline void write_settings(std::ostream& out,const Settings& s) {
    out<<"version=7\ngun_angle_zero_version=1\nview_defaults_version=4\np1_hand="<<s.p1_hand<<"\nreload="<<int(s.reload)<<"\ngun="<<(selectable_gun_choice(s.gun)?s.gun:AutoOriginalGun)<<"\nlaser="<<s.laser
       <<"\nlaser_width="<<s.laser_width<<"\nview_fov="<<s.view_fov<<"\nwider_view="<<s.wider_view<<"\nworld_scale="<<s.world_scale<<"\nview_size="<<s.view_size<<"\npanel_distance="<<s.panel_distance<<"\nhud_size="<<s.hud_size<<"\ngun_angle="<<s.gun_angle<<"\ngun_scale="<<s.gun_scale<<"\nreticle_mode="<<int(reticle_mode_value(int(s.reticle_mode)))<<"\nreticle_min_size="<<s.reticle_min_size<<"\nreticle_max_size="<<s.reticle_max_size<<"\nreticle_shrink_distance="<<s.reticle_shrink_distance<<"\nhaptics="<<s.haptics<<"\nbackground="<<int(s.background)<<'\n';
}
// Persist migration at the first real settings load, without a Settings ABI
// field or rewriting unrelated/unknown preferences and calibration entries.
inline Settings load_settings(const char* path) {
    std::ifstream file(path);if(!file)return Settings{};
    std::string text{std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};file.close();
    std::istringstream input(text);bool migrated=false,view_migrated=false;auto settings=read_settings(input,&migrated,&view_migrated);
    if(migrated||view_migrated) {
        std::istringstream old(text);std::ostringstream updated;std::string line;bool angle=false,marker=false,world=false,view=false,view_marker=false;
        while(std::getline(old,line)){
            auto key=line.substr(0,line.find('='));
            if(migrated&&key=="gun_angle"){updated<<"gun_angle=0\n";angle=true;}
            else if(migrated&&key=="gun_angle_zero_version"){updated<<"gun_angle_zero_version=1\n";marker=true;}
            else if(view_migrated&&key=="world_scale"){updated<<"world_scale=0.1\n";world=true;}
            else if(view_migrated&&key=="view_size"){updated<<"view_size=1\n";view=true;}
            else if(view_migrated&&key=="view_defaults_version"){updated<<"view_defaults_version=4\n";view_marker=true;}
            else updated<<line<<'\n';
        }
        if(migrated&&!angle)updated<<"gun_angle=0\n";if(migrated&&!marker)updated<<"gun_angle_zero_version=1\n";
        if(view_migrated&&!world)updated<<"world_scale=0.1\n";
        if(view_migrated&&!view)updated<<"view_size=1\n";
        if(view_migrated&&!view_marker)updated<<"view_defaults_version=4\n";
        std::ofstream output(path);if(output)output<<updated.str();
    }
    return settings;
}

}
