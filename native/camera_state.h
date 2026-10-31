// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>
namespace hotd_model2 {
template<class Read32> bool camera_revision_matches(Read32){return false;}
template<class Read32> uint32_t camera_write(uint32_t,uint32_t value,uint32_t,Read32,float){return value;}
struct NativeCameraZoom{float fov{},reference_fov{};};
template<class Read32> NativeCameraZoom read_native_camera_zoom(Read32){return {};}
}
