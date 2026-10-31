# Local build notes

Use MAME commit `f34f02505e32c1993c6a782b6814232cbfc74e36` in a new task-private Git repository. The engine must be pristine upstream before applying `patches/0001-native-hotd-host.patch`; `tools/sync_engine_patches.py` reconstructs and verifies all 16 patched files. It refuses unknown edits. Existing sibling engine/build trees are read-only references and are not compiler output directories.

Apply all four patches in numeric order through the build script. Patches2–4
add original-HOTD committed-shot observers and native trigger ownership capture.
Current engine and host must both use ABI `0x484f5403`; the older ABI2 binaries
cannot be mixed with them. The existing local signing key is preserved.

The current local compiler dependency is `D:/VR4ALL-HOTD-Model2-Build/engine`. It was prepared by initializing a new repository, fetching only the exact pinned upstream commit from local read-only Git objects with `--depth 1 --no-tags`, checking out detached and verifying HEAD. No sibling patches or build outputs were copied. Normal builds use the self-contained exported patch and do not need the optional `tools/export_common_engine.py` reference exporter.

Windows host and tests:

```powershell
./tools/test.ps1 -PythonPath D:/Python314/python.exe -MsysRoot D:/msys64
```

Quest engine, using the existing compiled SDL dependency read-only:

```powershell
./tools/build-engine.ps1 -MsysRoot D:/msys64 -PythonPath D:/Python314/python.exe -WorkEngine D:/VR4ALL-HOTD-Model2-Build/engine -Platform Android -AndroidNdk E:/AndroidSdk/ndk/29.0.14206865 -SdlRoot D:/StorageRelief/ege19/2026-10-25/vr4all-workspace-cache/carnevil-port/sdl-android -Jobs 1
```

Coordinate compiler use with the overseer. The build script defaults to one job and permits at most two. Set `TEMP` and `TMP` to a directory in this task's D: build root for additional compiler invocations. C: free space is limited. The build entry points preserve and restore their previous temp environment.

ARM64 host:

```powershell
E:/AndroidSdk/cmake/3.22.1/bin/cmake.exe -S . -B D:/VR4ALL-HOTD-Model2-Build/build/host-android -G Ninja -DCMAKE_MAKE_PROGRAM=E:/AndroidSdk/cmake/3.22.1/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=E:/AndroidSdk/ndk/29.0.14206865/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DANDROID_STL=c++_shared -DCMAKE_BUILD_TYPE=Release
E:/AndroidSdk/cmake/3.22.1/bin/cmake.exe --build D:/VR4ALL-HOTD-Model2-Build/build/host-android -j 1
```

Quest assembly requires the actual matching `libmain.so`, `libhotd_model2_xr.so`, SDL2, OpenXR loader and ARM64 shared C++ runtime. `tools/build-quest.ps1 -CheckOnly` does not build, install or publish. Without `-CheckOnly`, the script creates a new private development signing key if necessary, assembles the APK, verifies required ARM64 ELF libraries, rejects game/private data, verifies the signature and confirms package identity. The app's signing identity is independent of other ports.

Windows engine uses the same build script with `-Platform Windows`. The Windows host DLL alone is not a PCVR package. `tools/package.py` requires llvm-readobj and checks actual Windows AMD64 imports/exports plus media/private-file exclusion. It resolves dependencies beside the importing engine/host/loader and in the supplied MinGW runtime directory, follows transitive imports and rejects unresolved non-system DLLs. The final ZIP checker resolves against bundled components only and requires host exports `hotd_model2_xr_start`/`hotd_model2_xr_stop` plus loader `xrGetInstanceProcAddr`. Complete packaging needs the matching engine and OpenXR loader. No build entry point installs to a headset or publishes to GitHub.

Private fixture tests additionally use actual locally supplied program/RAM snapshots. Public source contains the test code and hashes, but not those copyrighted/private fixtures. Conditional CTest entries clearly distinguish tests available with private evidence from generic production checks.

Actual captured-game regressions use a complete private trace at `HOTD_MODEL2_GAMEPLAY_CAPTURE` (CMake cache FILEPATH, default `research/actual-scene-reference-v1/scenes/frame-00002303/metadata.json`) and `HOTD_MODEL2_TEST_PYTHON`. When available, `actual_hotd_host_geometry` loads the real host DLL in explicit GL simulation mode, renders a near raster frame then actual gameplay, and rejects a black world. `actual_hotd_scene_geometry` checks CPU-reference fidelity, poisoned depth/masks/scissor recovery, stereo eyes and head turn through production GLScene. `tools/replay_hotd_trace.py` creates temporary interchange files in the selected private output directory and keeps only logs/images there. No captured game data is public source.

The later frame-clear repair changes only host production `xr_host.cpp` and new `gl_frame.h`. ABI stays `0x484f5402`; rebuild both host libraries and packages, without recompiling the full engines. The new header can be mirrored with other dependency headers but is unused by engine translation units.

Review repair changes shared ABI to `0x484f5402`: copy every top-level native header including `start_policy.h` into the engine and rebuild both engines and hosts before packaging. `docs/review-header-manifest.json` records expected source hashes. Both real private-fixture CTest cases compile with assertions forced active; `native_state_test.cpp` rejects a build where NDEBUG survives.

```powershell
D:/Python314/python.exe -B tools/package.py --check-windows <hotd-model2-vr-pcvr.zip> --readobj E:/AndroidSdk/ndk/29.0.14206865/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-readobj.exe
```
