// SPDX-License-Identifier: GPL-2.0-or-later
// Pass-through GL draw observer: inspect the real renderer's actual materials.
#include "presentation.h"
#include "gl_context.h"
#include "shared/gun_model.h"
#include "shared/vr4all_splash.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace hotd_model2;
static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Draw {unsigned part;glm::vec4 base;glm::mat4 model;};
static std::vector<Draw> draws;
static PFNGLDRAWELEMENTSPROC original_draw;
static void GLAD_API_PTR observe_draw(GLenum mode,GLsizei count,GLenum type,const void* offset){
 GLint program{};glGetIntegerv(GL_CURRENT_PROGRAM,&program);
 const auto base=glGetUniformLocation(program,"base"),model=glGetUniformLocation(program,"model");
 if(base>=0&&model>=0)for(unsigned part=0;part<4;part++){
  const auto& expected=vr::xr::gunmodel::Parts[part];
  if(count==int(expected.count)&&uintptr_t(offset)==expected.first*4){
   Draw call;call.part=part;glGetUniformfv(program,base,&call.base[0]);glGetUniformfv(program,model,&call.model[0][0]);draws.push_back(call);
  }
 }
 original_draw(mode,count,type,offset);
}
static bool near(glm::vec4 a,glm::vec4 b){return glm::length(a-b)<1e-5f;}
int main()try {
 using namespace std::chrono;
 vr::xr::LaunchSplash splash;require(splash.active(),"Launch branding missing");
 splash.presented(nanoseconds(0));splash.presented(nanoseconds(-1));require(splash.active(),"Invalid/unpresented frame spent splash time");
 for(int i=0;i<29;i++)splash.presented(seconds(1));require(splash.active(),"Malformed periods did not clamp");
 splash.presented(milliseconds(100));require(!splash.active(),"Launch branding did not finish after three presented seconds");
 for(float displayed:{-30.f,0.f,30.f}){
  auto pose=glm::translate(glm::mat4(1),glm::vec3(.2f,.1f,-.3f));const auto actual=pitched_gun_pose(pose,displayed);
  const auto ray=glm::vec3(actual*glm::vec4(0,0,-1,0));const double radians=(displayed-10)*3.141592653589793/180.;
  require(glm::length(ray-glm::vec3(0,float(std::sin(radians)),float(-std::cos(radians))))<1e-5f,"Displayed gun angle does not apply the accepted minus10 baseline");
  require(glm::length(glm::vec3(actual[3])-glm::vec3(pose[3]))<1e-5f,"Angle adjustment moved the palm");
 }
 GLContext context;original_draw=glad_glDrawElements;glad_glDrawElements=observe_draw;
 SharedRuntime runtime;runtime.state.valid=runtime.state.ready=runtime.state.playing=true;
 for(auto& player:runtime.state.players){player.state=1;player.health=1;}
 Settings settings;settings.laser=false;Presentation ui;PresentationInput input;
 for(int h=0;h<2;h++){input.hands[h].tracked=true;input.poses[h]=glm::translate(glm::mat4(1),glm::vec3(h?.2f:-.2f,0,-.4f));}
 const auto vp=glm::perspective(glm::radians(90.f),1.f,.02f,100.f);
 for(int layout:{0,1})for(int choice:{0,8}){
  settings.p1_hand=layout;settings.gun=choice;ui.prepare(settings,&runtime);ui.update(input,settings,runtime,1);
  CabinetInput cabinet;cabinet.hand={layout,1-layout};cabinet.start_mask=1;ui.feedback(cabinet,1);
  draws.clear();glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  ui.draw(vp,settings,runtime.state,true,2);require(draws.size()==8,"Real Arcade/fallback guns did not draw all material groups for both players");
  for(unsigned player=0;player<2;player++)for(unsigned part=0;part<4;part++){
   const auto& call=draws[player*4+part];require(call.part==part,"Arcade semantic material ordering changed");
   const glm::vec4 expected=part<2?(player?glm::vec4(.04f,.25f,1,1):glm::vec4(.90f,.04f,.04f,1)):
     part==2?glm::vec4(.15f,.15f,.17f,1):glm::vec4(.12f,.22f,.3f,1);
   require(near(call.base,expected),part<2?"Arcade body/grip color does not follow logical player across selection/hand layout":"Screw/metal/lens material was recolored");
   const auto gpu_muzzle=glm::vec3(call.model*glm::vec4(0,.0471f,-.1401f,1));
   const auto pose=pitched_gun_pose(input.gun_pose(cabinet.hand[player]),settings.gun_angle);
   require(glm::length(gpu_muzzle-ui.muzzle(pose,settings,cabinet.hand[player]))<1e-5f,"Drawn model muzzle differs from aiming/laser/menu origin");
  }
 }
 // P2-only play retains P2's blue even when P1 is absent and the remaining
 // controller changes hands. Player identity must not collapse to draw index0.
 runtime.state.players[0].state=0;runtime.state.players[0].health=0;
 for(int hand:{0,1})for(int choice:{0,8}){
  settings.gun=choice;ui.prepare(settings,&runtime);ui.update(input,settings,runtime,3);
  CabinetInput cabinet;cabinet.hand={-1,hand};cabinet.start_mask=2;ui.feedback(cabinet,3);
  draws.clear();glDepthMask(GL_TRUE);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  ui.draw(vp,settings,runtime.state,true,4);require(draws.size()==4,"P2-only draw did not retain exactly its own gun");
  require(near(draws[0].base,{.04f,.25f,1,1})&&near(draws[1].base,{.04f,.25f,1,1}),"P2-only body/grip lost logical blue after a hand transfer");
 }
 require(glGetError()==GL_NO_ERROR,"Common presentation produced a GL error");glad_glDrawElements=original_draw;
 std::cout<<"Actual GPU Arcade/fallback body/grip, metal/lens, both logical-player hand layouts, model/muzzle, angle and launch-clock checks passed\n";
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
