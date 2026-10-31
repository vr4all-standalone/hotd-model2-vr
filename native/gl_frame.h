// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <glad/gl.h>

namespace hotd_model2 {
// Each simulation frame and XR eye owns a fresh color/depth target. Restore
// clear state explicitly: a preceding UI/raster pass may change these masks.
inline void begin_scene_frame(float red=0,float green=0,float blue=0,float alpha=1) {
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glDepthMask(GL_TRUE);
#ifdef __ANDROID__
    glClearDepthf(1.f);
#else
    glClearDepth(1.);
#endif
    glClearColor(red,green,blue,alpha);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}
}
