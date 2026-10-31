// SPDX-License-Identifier: GPL-2.0-or-later
// Actual privately captured HOTD geometry through the production GL renderer.
#include "captured_frame_io.h"
#include "gl_context.h"
#include "gl_frame.h"
#include "gl_scene.h"
#include "model2_scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

using namespace hotd_model2;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
glm::mat4 projection(const NativeLens& l) {
    constexpr float n=.001f;
    return glm::frustum(float(-l.cx/l.fx*n),float((l.width-l.cx)/l.fx*n),float(-(l.height-l.cy)/l.fy*n),float(l.cy/l.fy*n),n,10000.f);
}
size_t different(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b) {
    size_t result{};for(size_t i=0;i<a.size();i+=3)if(a[i]!=b[i]||a[i+1]!=b[i+1]||a[i+2]!=b[i+2])++result;return result;
}
size_t visible(const std::vector<uint8_t>& a) {
    size_t result{};for(size_t i=0;i<a.size();i+=3)if(a[i]||a[i+1]||a[i+2])++result;return result;
}
void ppm(const std::filesystem::path& path,int width,int height,const std::vector<uint8_t>& rgb) {
    std::ofstream out(path,std::ios::binary);out<<"P6\n"<<width<<' '<<height<<"\n255\n";
    out.write(reinterpret_cast<const char*>(rgb.data()),rgb.size());require(bool(out),"Cannot save actual scene replay");
}
}
int main(int argc,char** argv)try {
    if(argc!=5){std::cerr<<"usage: captured_scene_gl_test geometry.txt capture-dir unused-host.dll output-dir\n";return 2;}
    auto frame=test::read_frame(argv[1],argv[2]);const auto& f=*frame;
    require(Model2Resources(f).valid()&&f.polygons.size()>100,"Actual game resources/geometry unavailable");
    GLContext context;
    GLuint fbo{},color{},depth{};glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,f.width,f.height,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);
    glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,f.width,f.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
    require(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"Incomplete actual scene framebuffer");
    glViewport(0,0,f.width,f.height);GLScene scene;scene.prepare(frame);
    auto render=[&](glm::mat4 vp,bool poison=false) {
        if(poison) {
            glDisable(GL_SCISSOR_TEST);glDepthMask(GL_TRUE);glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClearDepth(0);glClearColor(1,0,1,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glEnable(GL_SCISSOR_TEST);glScissor(0,0,1,1);glDepthMask(GL_FALSE);glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
        }
        begin_scene_frame();scene.draw(vp,.02f,3.f);glFinish();
        std::vector<uint8_t> bottom(size_t(f.width)*f.height*3),rgb(bottom.size());
        glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,f.width,f.height,GL_RGB,GL_UNSIGNED_BYTE,bottom.data());
        for(int y=0;y<f.height;y++)std::copy_n(bottom.data()+size_t(f.height-1-y)*f.width*3,size_t(f.width)*3,rgb.data()+size_t(y)*f.width*3);
        require(glGetError()==GL_NO_ERROR,"Actual scene replay GL error");return rgb;
    };
    const auto vp=projection(presentation_lens(f));auto native=render(vp);
    std::vector<uint8_t> cpu;cpu.reserve(f.native.size()*3);
    for(auto pixel:f.native){cpu.push_back(pixel>>16);cpu.push_back(pixel>>8);cpu.push_back(pixel);}
    double error{};for(size_t i=0;i<cpu.size();i++)error+=std::abs(int(cpu[i])-int(native[i]));
    const double mae=error/cpu.size(),exact=1.-double(different(cpu,native))/f.native.size();
    std::cout<<"Captured HOTD CPU replay: polygons "<<f.polygons.size()<<", RGB MAE "<<mae<<", exact fraction "<<exact<<'\n';
    require(mae<=1.&&exact>=.98,"Actual captured-game CPU/GL replay fidelity failed");
    require(native==render(vp,true),"Actual captured-game replay changed with poisoned depth/masks/scissor");
    scene.set_immersive(true);auto neutral=render(vp);
    auto left=render(vp*glm::translate(glm::mat4(1),glm::vec3(.032f,0,0)),true);
    auto right=render(vp*glm::translate(glm::mat4(1),glm::vec3(-.032f,0,0)),true);
    auto turned=render(vp*glm::rotate(glm::mat4(1),glm::radians(25.f),glm::vec3(0,1,0)),true);
    const auto stereo=different(left,right),head_turn=different(neutral,turned);
    std::cout<<"Actual captured HOTD immersive pixels: neutral "<<visible(neutral)<<", left "<<visible(left)<<", right "<<visible(right)<<", turned "<<visible(turned)<<", stereo differences "<<stereo<<", head-turn differences "<<head_turn<<'\n';
    require(visible(neutral)>5000&&visible(left)>5000&&visible(right)>5000&&visible(turned)>5000,"Actual captured immersive world missing");
    require(stereo>500&&head_turn>500,"Actual captured stereo/head-turn response missing");
    const std::filesystem::path output=argv[4];std::filesystem::create_directories(output);
    for(const auto& [name,image]:{std::pair<const char*,const std::vector<uint8_t>&>{"cpu",cpu},{"neutral",native},{"immersive",neutral},{"left",left},{"right",right},{"turned",turned}})
        ppm(output/(std::string(name)+".ppm"),f.width,f.height,image);
    glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&color);glDeleteRenderbuffers(1,&depth);return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
