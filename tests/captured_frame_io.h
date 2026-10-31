// SPDX-License-Identifier: GPL-2.0-or-later
// Selected diagnostic frame reader adapted from the licensed VC2 replay tool.
// Geometry/resources are generated privately from owned HOTD media, not shipped.
#pragma once
#include "model2_capture.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace hotd_model2::test {
inline std::vector<uint8_t> read_bytes(const std::filesystem::path& file) {
    std::ifstream in(file,std::ios::binary);
    if(!in)throw std::runtime_error("Cannot read "+file.string());
    return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
}
inline std::shared_ptr<NativeFrame> read_frame(const char* text,const std::filesystem::path& root) {
    auto f=std::make_shared<NativeFrame>();std::ifstream in(text);
    if(!in)throw std::runtime_error("Cannot read diagnostic geometry");
    size_t count{};in>>f->sequence>>f->width>>f->height>>f->xoffset>>f->yoffset>>count;
    if(count>32768||f->width<1||f->height<1||f->width>2048||f->height>2048)throw std::runtime_error("Invalid diagnostic frame");
    f->polygons.resize(count);
    for(auto& p:f->polygons) {
        in>>p.count>>p.attr>>p.window>>p.sort>>p.luma>>p.lod>>p.object>>p.culled;
        if(p.count<3||p.count>4)throw std::runtime_error("Invalid diagnostic polygon");
        for(auto& h:p.header)in>>h;
        for(auto& h:p.focus)in>>h;
        for(auto& h:p.center)in>>h;
        for(auto& h:p.viewport)in>>h;
        for(unsigned i=0;i<p.count;i++){auto& v=p.vertices[i];in>>v.x>>v.y>>v.z>>v.u>>v.v;}
    }
    if(!in)throw std::runtime_error("Incomplete diagnostic geometry");
    std::ifstream culls(std::string(text)+".culls");
    for(auto& p:f->polygons)culls>>p.cull_reasons;
    if(!culls)throw std::runtime_error("Incomplete diagnostic cull metadata");
    for(const auto* name:{"texture0","texture1","palette","colorxlat","luma","gamma"})
        f->blobs.push_back({name,read_bytes(root/(std::string(name)+".bin"))});
    for(const auto* name:{"background","foreground","native"}) {
        auto raw=read_bytes(root/(std::string(name)+"-argb32.bin"));
        if(raw.size()!=size_t(f->width)*f->height*4)throw std::runtime_error("Incomplete native raster");
        auto& pixels=std::string(name)=="background"?f->background:std::string(name)=="foreground"?f->foreground:f->native;
        for(size_t i=0;i<raw.size();i+=4)pixels.push_back(raw[i]|(uint32_t(raw[i+1])<<8)|(uint32_t(raw[i+2])<<16)|(uint32_t(raw[i+3])<<24));
    }
    f->complete=true;return f;
}
}
