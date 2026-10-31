# The House of the Dead VR credits

The House of the Dead and its artwork, audio and game code belong to their respective owners. Game data is supplied by the user and is excluded from the APK and software packages.

The emulator is MAME 0.289, commit `f34f02505e32c1993c6a782b6814232cbfc74e36`, by the MAME developers. MAME source files retain their GPL and BSD notices. The Model 2 replay rules follow the original BSD-licensed renderer by R. Belmont, Olivier Galibert, ElSemi, Angelo Salese and Matthew Daniels. See `LICENSE` (upstream `COPYING`) and the notices in the pinned source files. The modifications and new host code use GPL-2.0-or-later.

The VR4ALL panel/theme, GLB reader and baked Arcade gun were adapted from mikermak's Death Crimson OX / hotd2-vr source, local reference commit `66ed916b95d1fb855bc3a31f1ff9830bd068413a`. Their original notices remain in `native/shared/`.

The shaped muzzle flame/star and drifting gun smoke in `native/gun_effects.h` are adapted from mikermak's VR4ALL Confidential Mission `xr_gun.cpp` (GPL-2.0-or-later, Copyright 2026 mikermak).

The bounded private diagnostic frame reader/text interchange in `tests/captured_frame_io.h` and `tools/replay_hotd_trace.py` selects GPL-2.0-or-later logic from the existing VR4ALL VC2 diagnostic replay tools. Reference source hashes are recorded in `docs/render-repair.json`. Captured game geometry, resources and images remain private and are excluded from public source/packages.

- “Namco Arcade Gun” by Martoscar, CC BY 4.0. [Original model](https://sketchfab.com/3d-models/namco-arcade-gun-15fbd5b9add94a34b5c21746e3dd32be). Baked positions/normals and material colors adapted by mikermak; original textures omitted.
- “Nintendo Zapper” by BIGDOGLOBAL (bdglobal), CC BY-ND 4.0. [Original model](https://sketchfab.com/3d-models/nintendo-zapper-e2a445cbf957439ab3a3f0c0408cd422). The existing GLB is included unchanged; its existing conversion, real-size scale and texture resizing are documented in `assets/models/CREDITS.txt`.
- “Meta Quest Left Controller” and “Meta Quest Right Controller” by declan02530, CC BY 4.0. Original model links and conversion changes appear in `assets/models/CREDITS.txt`.

Other components retain their source notices: Khronos OpenXR SDK 1.1.43 and loader (Apache 2.0 and included third-party licenses), SDL 2.33.0 development source (zlib), GLM (MIT), glad generated bindings (public-domain/CC0 generation), cgltf (MIT), stb_image and stb_truetype (MIT/public-domain), and Roboto Bold (Apache 2.0). Gradle and the Android tools are build dependencies.

This VR4ALL community port is not an official MAME or Sega product.

Original HOTD gun geometry has not yet been verified or offered. Arcade and the unchanged Zapper are the initial selectable models.
