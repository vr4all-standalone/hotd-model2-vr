// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <array>
namespace hotd_model2 {
// Displayed zero is the former -10 degree pose. The caller supplies the
// existing palm/aim pivot; every gun, muzzle, menu ray and native aim shares it.
inline glm::mat4 pitched_gun_pose(const glm::mat4& pose,float displayed_angle) {
    const float angle=std::isfinite(displayed_angle)?std::clamp(displayed_angle,-30.f,30.f):0.f;
    return pose*glm::rotate(glm::mat4(1),glm::radians(angle-10.f),glm::vec3(1,0,0));
}

// Original hotdo PN_ter00a: measured authored hand volume centroid and front
// barrel-cap centroid, retained in native source units (see weapon evidence).
inline glm::vec3 native_gun_grip(unsigned weapon) {return weapon==1?glm::vec3(-.1287351734f,-.5152499676f,-.0431462593f):glm::vec3(0);}
inline glm::vec3 native_gun_muzzle(unsigned weapon) {return weapon==1?glm::vec3(-.3750330769f,-2.2581366301f,.6543127969f):glm::vec3(0);}
inline glm::mat4 native_gun_placement(unsigned weapon) {
    // Source -Y is the barrel and +Z is its top. Map (x,y,z) -> (-x,z,y),
    // then use a deliberate 0.1m/source-unit cosmetic presentation scale.
    return glm::rotate(glm::mat4(1),glm::pi<float>(),glm::vec3(0,0,1))
        *glm::rotate(glm::mat4(1),glm::half_pi<float>(),glm::vec3(1,0,0))
        *glm::scale(glm::mat4(1),glm::vec3(.1f))
        *glm::translate(glm::mat4(1),-native_gun_grip(weapon));
}
// The aim origin is above/in front of the palm. Keep its barrel orientation,
// but anchor the model to the runtime's physical grip position.
inline glm::mat4 gun_pose(glm::mat4 aim,const std::optional<glm::vec3>& grip) {
    if(grip)aim[3]=glm::vec4(*grip,1);
    return aim;
}
inline glm::mat4 gun_model_transform(const glm::mat4& pose,float scale,int hand,const glm::mat4& placement,bool original_hand=false) {
    // Reflect in controller space AFTER aligning the authored grip and barrel.
    // The original game's embedded hand needs the opposite assignment from
    // Arcade/Zapper. Keep its palm and barrel axis fixed when swapping sides.
    const bool mirrored=original_hand?hand==1:hand==0;
    return pose*glm::scale(glm::mat4(1),glm::vec3(mirrored?-scale:scale,scale,scale))*placement;
}
}
