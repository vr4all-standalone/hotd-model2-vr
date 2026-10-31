// SPDX-License-Identifier: GPL-2.0-or-later
#include "../native/controller_controls.h"
#include <cstdlib>
#include <iostream>
using namespace hotd_model2;
static void require(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
int main(){
 Settings settings;settings.world_scale=0.110f;Hands hands;for(auto& h:hands)h.tracked=true;
 ExtraControls input;ShortcutPolicy policy;policy.update(1,input,hands,settings,true);
 for(int hand=0;hand<2;hand++)for(auto stick:{Stick{0,-1},Stick{0,1},Stick{-1,0},Stick{1,0}}){
  input={};policy.update(2,input,hands,settings,true);input.grip[hand]=true;
  policy.update(2.1,input,hands,settings,true);input.stick[hand]=stick;
  for(int n=0;n<20;n++){
   auto result=policy.update(2.2+n*.1,input,hands,settings,true);
   require(settings.world_scale==0.110f&&settings.gun_scale==1&&settings.view_size==1&&!settings.game_zoom,"Grip still changes world/gun/view settings");
   require(!result.save,"Grip still requests saving a setting");
  }
 }
 input={};policy.update(8,input,hands,settings,true);input.click[0]=true;
 require(policy.update(8.1,input,hands,settings,true).recenter,"Removing grip also removed the left-stick recenter click");
 input={};policy.update(8.2,input,hands,settings,true);input.click[1]=true;
 require(policy.update(8.3,input,hands,settings,true).laser,"Removing grip also removed the right-stick laser click");
 policy.suspend();require(!policy.update(8.4,input,hands,settings,false).laser,"Focus loss reused a stick click");
 std::cout<<"Grip+all stick axes inert for both hands; recenter/laser and focus guards retained\n";
}
