// SPDX-License-Identifier: GPL-2.0-or-later
// Original geometry/resources are supplied privately at runtime, never shipped.
#include "presentation.h"
#include "gl_context.h"
#include "native_weapon.h"
#include "captured_frame_io.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace hotd_model2;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static bool close(glm::vec3 a,glm::vec3 b){return glm::length(a-b)<1e-5f;}
static uint32_t word(const std::vector<uint8_t>& data,size_t at,unsigned n=4){require(at+n<=data.size(),"Truncated private original-gun fixture");uint32_t out=0;for(unsigned i=0;i<n;i++)out|=uint32_t(data[at+i])<<(8*i);return out;}
int main(int argc,char** argv)try {
 require(argc==3,"usage: original_gun_presentation_test private-weapon-fixtures private-scene-capture");
 require(close(native_gun_grip(1),{-.1287351734f,-.5152499676f,-.0431462593f}),"Original palm does not use the measured authored hand centroid");
 require(close(native_gun_muzzle(1),{-.3750330769f,-2.2581366301f,.6543127969f}),"Original muzzle does not use its actual barrel cap");
 const std::filesystem::path fixture=argv[1],capture=argv[2];
 auto program=test::read_bytes(fixture/"program.bin"),data=test::read_bytes(fixture/"main-data.bin"),polygons=test::read_bytes(fixture/"polygons.bin"),textures=test::read_bytes(fixture/"textures.bin");
 auto native=std::make_shared<NativeWeapon>(extract_native_weapon([&](uint32_t at){return at>=0x2000000?word(data,at-0x2000000):word(program,at);},[&](uint32_t at){return word(polygons,size_t(at)*4);},[&](uint32_t at){return word(textures,size_t(at)*2,2);},1,uint32_t(polygons.size()/4),uint32_t(textures.size()/2)));
 require(native->valid&&native->model==0x719&&native->polygons.size()==50,"Actual original HOTD pistol was not extracted");
 auto frame=std::make_shared<NativeFrame>();for(const char* name:{"texture0","texture1","palette","colorxlat","luma","gamma"})frame->blobs.push_back({name,test::read_bytes(capture/(std::string(name)+".bin"))});
 GLContext context;SharedRuntime runtime;runtime.weapons[1]=native;runtime.state.valid=runtime.state.ready=runtime.state.playing=true;for(auto& p:runtime.state.players){p.state=1;p.health=1;}
 Settings settings;settings.gun=8;settings.laser=false;Presentation ui;ui.prepare(settings,&runtime);
 // Mapping source (x,y,z) to controller (-x,z,y) at 0.1m/source-unit
 // puts the authored palm at the grip and the cap 174.289mm down the barrel.
 const glm::vec3 measured{-.02462979f,.06974591f,-.17428867f};
 require(!close(ui.muzzle(glm::mat4(1),settings,1),measured),"Original option did not use its safe fallback without materials");
 runtime.scene.store(frame);ui.prepare(settings,&runtime);
 require(close(ui.muzzle(glm::mat4(1),settings,1),measured),"Original cache/axis/physical-scale placement did not reach the measured barrel cap");
 require(close(ui.muzzle(glm::mat4(1),settings,0),{.02462979f,.06974591f,-.17428867f}),"Original left/right hand meshes were not swapped around their palms");
 auto aim=glm::translate(glm::mat4(1),glm::vec3(.2f,.1f,-.3f));auto palm=glm::vec3(.15f,.05f,-.25f);
 const auto pose=pitched_gun_pose(gun_pose(aim,palm),10);
 require(close(ui.muzzle(pose,settings,1),palm+measured),"Original muzzle bypassed runtime grip anchoring or the shared angle pivot");
 settings.gun_scale=.5f;require(close(ui.muzzle(pose,settings,1),palm+measured*.5f),"Gun-size adjustment moved the palm instead of scaling around it");settings.gun_scale=1;
 GLuint fbo{},color{},depth{};glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,256,256);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);require(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"Original-gun framebuffer incomplete");glViewport(0,0,256,256);
 PresentationInput input;for(int h=0;h<2;h++){input.hands[h].tracked=true;input.poses[h]=glm::translate(glm::mat4(1),glm::vec3(h?.2f:-.2f,0,-.4f));}ui.update(input,settings,runtime,1);CabinetInput cabinet;cabinet.hand={0,1};cabinet.start_mask=1;ui.feedback(cabinet,1);
 const auto vp=glm::perspective(glm::radians(90.f),1.f,.02f,100.f);
 auto render=[&](){glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);ui.draw(vp,settings,runtime.state,true,2);std::vector<uint32_t> pixels(256*256);glReadPixels(0,0,256,256,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());require(glGetError()==GL_NO_ERROR,"Original-gun renderer GL error");return pixels;};
 const auto original=render();size_t visible=0;std::set<uint32_t> colors;for(auto px:original)if((px&0xffffff)!=0){visible++;colors.insert(px&0xffffff);}require(visible>100&&colors.size()>16,"Actual Original option did not render its textured native mesh");
 settings.gun=0;const auto arcade=render();size_t changed=0;for(size_t i=0;i<original.size();i++)changed+=original[i]!=arcade[i];require(changed>100,"Original option still rendered the Arcade fallback");
 std::cout<<"Actual original pistol presentation: "<<visible<<" visible pixels, "<<colors.size()<<" colors, "<<changed<<" pixels distinct from Arcade; measured palm/muzzle, left hand and grip-scale passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
