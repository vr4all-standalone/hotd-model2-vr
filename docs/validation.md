# HOTD Model 2 validation — 2026-10-29

## Current captured defaults:0.1.7-dev

The live Quest menu on2026-10-30 was captured without input or interruption.
Its only changed visible default is Reticle maximum75% (formerly100%).
Custom22% minimum /6.7m shrink, LaserOn, Original pistol, angle0, Shoot off-screen,
P1left/P2right, HapticsMedium, world0.10/View100%, panel1.80m, HUD100% and gun1.00
remain. New/missing/invalid maximum and Reset use75%; explicit valid choices
persist without forced migration. HOTD2 now matches the complete22% minimum /75% maximum /6.7m profile
by the user's subsequent explicit instruction. See workspace
`overseer/changes/2026-10-30-hotd-captured-defaults/report.md`.

## Historical scale correction:0.1.6-dev

World scale now defaults to0.10 in HOTD and HOTD2. The0.102 option is removed;
menu steps are uniformly0.01 (0.10,0.11,0.12). HOTD range0.01–2.00 and HOTD2
range0.01–0.20; View100% retained in HOTD. Migration markers4/3 apply once to
older settings, preserving other preferences and later choices. Evidence is
in `overseer/changes/2026-10-30-hotd-both-scale100/report.md` and
`docs/final-build-status.json`. Startup/physical acceptance remains separate.

## Historical added-scale update:0.1.5-dev

An exact **0.102** world-scale option is added between the retained0.100 and
0.105 options, and becomes the default in HOTD and HOTD2. This follows the user's
earlier decimal convention; the working interpretation of this request's1.02
was stated as0.102 before implementation. HOTD keeps View size100%. Marker3
applies the new world/view baseline once to older settings, including marker2
files, preserving other settings and subsequent manual choices. Approved common
UI/label/colors and native controls remain intact. Fresh verification evidence
is in `docs/final-build-status.json` and the workspace batch
`overseer/changes/2026-10-29-hotd-both-scale102/report.md`.
Candidates: `D:/VR4ALL-HOTD-Model2-Build/artifacts/scale0102/`.
Physical/startup acceptance and PCVR headset play remain separate checks.

## Historical common audit/naming candidate:0.1.4-dev

The Quest application label is **The House of the Dead - VR4ALL**, preserving
package/signing identity. The wide validated setup, auto-entry/manual recovery,
shared purple menu/help and three-presented-second launch branding were already
implemented. Setup palette/880dp content width match the approved HOTD2 launcher;
the manifest retains its wide860x640dp resizable Quest panel.

A confirmed gap is fixed: Arcade fallback models while Original loads now use
logical P1 red/P2 blue just like explicit Arcade selection. Only body/grip semantic
parts are colored; screws/metal/lens and actual native/Zapper models retain their
materials. A pass-through observer checks actual production GL draw uniforms,
both player/hand layouts, fallback/Arcade selections, and model/muzzle agreement.
It also checks accepted angle poses and the bounded presented-frame splash clock.

The complete workflow passes39production Python/Java/native checks and22CTests.
Existing angle tests cover the one-time legacy reset and later preference
persistence. World0.105/View100%, no-grip shortcuts and corrected Original hands
remain intact. Current source/build evidence is in `docs/final-build-status.json`
and `overseer/changes/2026-10-29-hotd-common-audit/report.md` in the workspace.
Candidate packages use `D:/VR4ALL-HOTD-Model2-Build/artifacts/common-audit/`.
This candidate is not installed or published; headset use is reserved for the
overseer batch. Actual setup/menu appearance, aiming/audio/haptics/performance,
complete native routes, semantic HUD separation and PCVR play remain physical
or native-validation gates. No deferred settings-overhaul work was added.

## Historical scale update:0.1.3-dev

The user selected **World scale0.105** for HOTD and HOTD2. HOTD keeps View size100%.
Marker `view_defaults_version=2` applies0.105/100% once, including settings from
the installed0.110 baseline, while preserving other settings and later choices.
Exact0.105 menu selection, real file persistence/migration and the retained
grip-removal/gun-hand behavior pass39production checks and21CTests. Current
build/package/install evidence is in `docs/final-build-status.json` and the
workspace batch `overseer/changes/2026-10-29-hotd-both-scale/report.md`.
Artifacts: `D:/VR4ALL-HOTD-Model2-Build/artifacts/scale0105/`.
Headset appearance, audio/performance acceptance, complete routes and physical
PCVR remain separate gates. No publication authorized.

## Historical preferences update: 0.1.2-dev

The user explicitly confirmed **0.110** after the actual headset log showed
0.114617 (menu0.115). The earlier literal1.10 staging was never installed.

VersionCode3, package/ABI/signing identity unchanged. User physically confirms
the world scale control produces the desired effect and selects **World scale
0.110 / View size100%**. Fresh defaults use these values. Old settings migrate
once using `view_defaults_version=1`; unrelated preferences and unknown/native
calibration entries are preserved. Subsequent explicit scale/view choices save
and load normally. The menu offers exact0.110/0.115 choices (0.005 steps at/below
0.20,0.05 above, range0.010–2.000), displayed to three decimals.

Squeeze button action/bindings/polling and grip+stick size controls/help are
removed. World/gun size remain menu controls. Tracking the physical palm pose
is retained for correct gun anchoring. Stick-click recenter/laser behavior and
fresh-input guards remain. The Original pistol's hand mirroring is swapped on
both controllers; drawing, muzzle/aim and effects use the same corrected
transform. Arcade/Zapper mirroring remains unchanged.

**39 production checks and21 CTests pass**, including actual settings migration,
exact menu steps, inert grip samples for both hands/all axes, retained clicks,
and real Original pistol extraction/GPU presentation with corrected muzzles.
The unchanged native engine algorithms retain the previous strobe/performance
fixes. Current build/install identities are in `docs/final-build-status.json`.
Physical acceptance of this hand/defaults update is a separate pending gate.
Current candidates use `D:/VR4ALL-HOTD-Model2-Build/artifacts/preferences/`.

## Historical playtest update: 0.1.1-dev

VersionCode 2, package `com.vrlightgunhub.hotdmodel2`, shared ABI `0x484f5403`.
The user physically played the initial Quest 3S build and reported it worked
fairly well, with slow emulation/elongated audio, a sensor flash and enlarged
presentation. This update is installed over it with app data and signing identity
preserved; installed APK SHA256 matches the candidate. The headset was asleep
during the update, so physical launch/play/audio acceptance of this version is
pending. PCVR physical testing remains pending. No publication is authorized.

Android now discovers the fastest CPUs already allowed to the emulation thread.
Missing sysfs data or an affinity error preserves existing scheduling. Native
game clocks/gameplay are unchanged. Old headset logs showed late gameplay at
87–93% with audio underruns. An isolated native Quest benchmark reproduced
81–85%; manually selecting the faster cluster reached 115–123%. The final
production engine automatically selected mask `0x3c` from `0x3f`
(`applied=1,error=0`); final samples were 122.816%, 116.887%, 116.946%.
These bounded benchmarks used synthetic hands, no OpenXR session and no audio
output. They establish CPU headroom, not heard audio or headset FPS acceptance.

The VR renderer suppresses the verified original cabinet lightgun sensor strobe
using a per-draw primary tile-window mask. ROM fingerprints, native requests,
cooldown and all-ones mask gate it; no RAM/input/shot/audio changes. Real native
tile drawing functions pass primary/secondary mask, HUD marker and restoration
checks; white transitions/service/unknown ROMs retain native behavior.
Two fresh packaged 50-native-second runs confirm the displayed result: with
the diagnostic bypass, all three sampled sensor-strobe PNGs contain one pale
color; with suppression, the three contain 616/681/680 colors and visible
gameplay/credit text. Native mask RAM remains all ones in both cases. The
observation-only script changes no native memory. These are CPU reference
renderer checks, separate from physical headset acceptance.

Original is now a selectable textured PN_ter00a pistol, extracted at runtime
from the user's verified game data. Production extraction and GPU tests cover
palm/muzzle/angle/scale, left mirroring, fallback and retry. It is a cosmetic
tracked pistol; no new equipment/pickup mechanic is claimed. No game mesh/texture
is shipped. Existing gun preferences remain; choose **Gun → Original** to switch.

**View size**, 50–100%, changes angular scene framing with matching stereo/sky
and inverse aim transforms. Try 75–80% for smaller framing. **World scale
(depth)** changes depth; **Panel/HUD distance** places the raster/HUD surface.
Arcade coordinates have no verified metre calibration. Grip help now says hold
Grip, stick up/down World, left/right Gun. Unsupported wider-view, view-width
and game-zoom rows are hidden. Settings version 7 preserves explicit choices
and passes save/load, migration and invalid-value checks. HOTD2 was a read-only
scale reference; the all-games settings pass remains deferred.

The final workflow passes **38 Python/Java/native/PE checks and 20 CTests**.
Matching Windows/ARM64 engines and both hosts built; signed ARM64 Quest APK and
dependency-complete media-free PCVR ZIP are in
`D:/VR4ALL-HOTD-Model2-Build/artifacts/playtest-fixes/`.
A fresh packaged 90-native-second run independently observed **6 P1 / 6 P2
committed shots**, matching host totals and all 12 software feedback events.
Twelve refunded attempts, two reloads and 36 dry attempts produced no extra
accepted events. Production input policy drove synthetic hands; Lua only
observed memory. No physical haptics/audio claim follows from this test.

Current identities/evidence are in `docs/final-build-status.json` and the
workspace's `overseer/changes/2026-10-29-hotd-playtest-fixes/report.md`.
Physical acceptance of updated Quest rendering/aim/audio/haptics/performance,
PCVR headset checks, complete routes/death/continue/P2 calibration, semantic HUD
separation and a wider native camera remain unverified. Upstream
`MACHINE_NOT_WORKING` remains. **Release readiness: false.**

## Historical native-feedback checkpoint (0.1.0-dev)

The sections below preserve earlier results and limitations; the current update
above supersedes their build paths, counts, gun availability and install status.

The current engine/host interface is **ABI `0x484f5403`**. New read-only HOTD
observers track the committed shot flag, rather than the provisional ammo store
which can be refunded. Feedback tokens bind each shot to its native cabinet
trigger edge and consumed candidate, so delayed publication, hand transfer,
tracking recovery and menu/focus changes cannot move pending recoil to a new
owner. Native totals and per-ownership feedback counts are separate.

The final workflow passes **34 Python/Java/native/PE tests and 15 CTests** (exit0),
including real original-ROM state and captured-game geometry fixtures. Matching
Windows/ARM64 engines and both hosts built; the Quest APK passes signature and
identity checks using the preserved local key. Current candidates are under
`D:/VR4ALL-HOTD-Model2-Build/artifacts/native-shots/`.

The packaged runtime passed a 90-native-second run with synthetic hands and an
independent observation-only Lua script: native and host totals both report
**11 P1 and 11 P2 accepted shots**, with all22 reaching the software feedback
path. Seven P1/two P2 refunded attempts, two reloads per player, and19 P1/15 P2
empty-gun attempts did not create extra accepted events. The run used only
packaged runtime dependencies, disabled audio and created no headset session.
Reproducer: `tools/verify_native_shots.py`; results: `research/native-shots-live-v1/result.json`.

The new regression was observed failing before implementation and on tracking
loss, then passed with delayed publication and an intervening native input
change covered. Final source/build/package identities are recorded separately in
`docs/native-shots-status.json`. Physical haptics and headset play remain untested.
The ABI2 evidence below is the preserved initial milestone.

Implemented and tested the native original-revision HOTD adapter and review repairs. Parent independently verified **33 Python/Java/native/real-PE tests and 14/14 CTest, exit 0**, including active native fixtures and captured-game rendering. Both ABI2 engines and both repaired hosts are built, and signed Quest APK/media-free PCVR candidates are prepared. The packaged engine/host passed a 60-native-second software run with both native players and complete rendered gameplay. ABI remains `0x484f5402`. **These are development artifacts; physical playability, headset acceptance and publication are not claimed.**

## Identity and scope

- Source: `ports/hotd-model2-vr`; exclusive task-private dependency/output root `D:/VR4ALL-HOTD-Model2-Build`. Source `build/`, `private/`, `research/` are verified junctions to this root.
- MAME 0.289 exact upstream commit `f34f02505e32c1993c6a782b6814232cbfc74e36`. A new shallow exact-commit upstream repository was prepared; no sibling fork/build tree was copied. Its selected 16 file exported patch is `patches/0001-native-hotd-host.patch` (SHA256 in `docs/artifacts.json`).
- Original **hotdo** is the supported initial program, Model 2C / i960KB / MB86235 TGPx4 / SCSP. Supplied legacy `hotd.zip` is not MAME's newer Revision A. The newer revision has a separate hash contract and no assumed runtime compatibility.
- App `com.vrlightgunhub.hotdmodel2`, versionCode 1 / `0.1.0-dev`; library `hotd_model2_xr`, current ABI `0x484f5403`, `HOTD_MODEL2_*` environment, independent settings/NVRAM. Signing configuration preserves private `hotd-model2.keystore`, alias `hotdmodel2`.
- Canonical current local packages: `D:/VR4ALL-HOTD-Model2-Build/artifacts/native-shots/hotd-model2-vr-quest.apk` and `hotd-model2-vr-pcvr.zip`, with adjacent `SHA256SUMS.txt`/`artifact-validation.json`. The earlier ABI2 candidates remain in `artifacts/review-fixed/`. Release repository is unset; publication is not authorized. The installer and existing third-party HOTD entry are untouched.

## Media and actual native evidence

The user's read-only source is `<local-development-tree>/confidential mission/Roms/Sega Model 2 Emulator v1.1a/Roms/hotd.zip`. All 29 payloads pass size, CRC32 and SHA1; no separate board/billboard firmware is required by this original revision's actual MAME list. Original archive SHA256 `3f3f7e7f640a19077483326f62391ab602c2daa6c03550067b42f3cca0551402` is unchanged. Private canonical `hotdo.zip` SHA256 `119d73ff88aace2a08a5ee0f26b2bfc5c32eea613da21393e2b7aaf7f9e07144`.

An existing read-only MAME executable was used for dependency-independent native evidence, with all VR environment flags removed, `-video none -sound none -noreadconfig`, hidden/no headset and isolated state directories. Executable SHA256 `104a0e6af2ba9996f4e6ab89109f7e42a13048aad01c852b8a95cfc3d8ebe6d5`. Actual privately dumped original program SHA256 `da2315b0b044d279728c8689336da6de0fee5997cf006514a1a633bd0de2fc75`.

- `-rompath <private/roms> -verifyroms hotdo`: exit 0, `romset hotdo [hotd] is good`. `-listroms hotdo`: exit 0. Exact command/output saved in `research/native-media-cli.json`.
- `python -B tools/run_native.py --engine <read-only-reference-vcop2.exe> --roms <private/roms> --output <research/native-original-v1> --seconds 35`: exit 0, no Lua errors. Native boot/title/credit/mission screens observed.
- Same runner with `--script tools/trace_native.lua --seconds 70`: exit 0, no Lua errors. Two native credit pulses at 16/17 seconds then P1 Start 18.5; P2 credits 34/35 then native join 36.5. Actual P1/P2 HUD/gameplay, ammo depletion and offscreen reload observed. Default native Start price 2, Continue price 1, shared credit pool confirmed from code and runtime.
- `--script tools/service_probe.lua --seconds 35`: exit 0. Actual Test Menu and Gun Setting menu observed. `--script tools/calibration_probe.lua --seconds 50`: exit 0; Gun Mark Checking and P1 Gun Adjustment observed.
- `--script tools/service_exit_probe.lua --seconds 72`: exit 0. P1 adjustment exited through Gun Setting and Test Menu; native Service flag cleared at 66 seconds, native reset mode 0 followed by attract mode 9 at 70 seconds. All saves/captures remain private; original media/state unchanged.

These establish native software behavior. Audio was disabled for these diagnostics and has not been listened to. This reference executable is not shipped as this port's engine.

## Production verification

Current command `./tools/test.ps1` completed **exit 0**, recorded in `research/geometry-debug-v1/full-tests.log`: **33/33 Python/Java/native/real-PE cases and 14/14 CTest**. The command builds the actual Windows host with Release CMake/MinGW and `-j 1`, using private D: temporary/output paths. Cases cover fractional/delayed credit accounting, bounded acknowledgement and repricing, shared-pool contention, P2-only hand/Continue transitions, actual PE/DLL packaging closure/exports, and actual captured-game rendering. Earlier policy repair evidence remains in `docs/review-fixes.json`; current rendering evidence is `docs/render-repair.json`.

Review exposed that the earlier CTest `native_state_test` target had `-DNDEBUG` without `-UNDEBUG`; its two private-fixture CTest entries therefore previously ran with assertions disabled. A new compile guard reproduced this as a failing actual target before the repair. The target now explicitly undefines NDEBUG and the source rejects any disabled-assertion build. **Both actual original-program/RAM and service/calibration fixture CTests pass with assertions active.** Earlier standalone fixture compilation used explicit `-UNDEBUG`; that does not validate the old CTest target. Counts and host hashes in the following paragraphs are retained as initial-build history, not current ABI2 candidate identity.

`D:/Python314/python.exe -B -m unittest discover -s tests -v`: **26/26 pass, exit 0**. Includes compiled actual C++ cabinet/state/input/settings checks; Java production media/lifecycle harnesses including the full owned 29 payload archive; ZIP corruption/duplicate/traversal/encryption cases; package private/media exclusion; and a real engine-source native-identity regression. Parent independently reran26/26 with exit 0.

`tools/test.ps1` configures/builds the Windows OpenXR host and invokes CTest. **10/10 CTest pass, exit 0**: cabinet, unknown revision boundary, input/settings, immutable capture, frame pool, angle migration, camera-space geometry, actual OpenGL head-turn/behind-camera/lean/depth checks, real original HOTD program/RAM reader fixtures, and real service/calibration fixtures. Parent independently ran the original25test suite plus all 10 CTest with exit 0; the later26th test covers the repaired native MAME identity regression. Final combined log `research/final-tests-v2.log`, exit 0 (26/26 Python/Java/native and 10/10 CTest).

Windows host build command used CMake 3.22.1/Ninja, MinGW GCC 15.2, C++ 20, Release, `-j 1`, isolated `build/host-windows`. Final `hotd_model2_xr.dll` SHA256 `c8d29969050bdef365da9013937a8751b00e67e86535ad02637630c8ea42f599`. One existing upstream stb_image GCC warning was emitted; no local host build error remains.

ARM64 host used NDK 29.0.14206865, Android 29, `arm64-v8a`, `c++_shared`, CMake/Ninja Release, `-j 1`, isolated `build/host-android`. Initial and final state/calibration refresh builds exit 0. Final `libhotd_model2_xr.so` SHA256 `9d4ffd4cf22e08e2678f388b5dd9d607ca0a3cf991c68d3ca8bdc3c19ccb09fc`.

## Full engine history and current integration ownership

Exact entry point:

```powershell
./tools/build-engine.ps1 -MsysRoot D:/msys64 -PythonPath D:/Python314/python.exe -WorkEngine D:/VR4ALL-HOTD-Model2-Build/engine -Platform Android -AndroidNdk E:/AndroidSdk/ndk/29.0.14206865 -SdlRoot D:/StorageRelief/ege19/2026-10-25/vr4all-workspace-cache/carnevil-port/sdl-android -Jobs 1
```

1. `research/engine-android-build.log`: wrapper exit 1 / make exit 2 before compilation. A one-time export used platform-default CP1252, producing en-dash byte 0x96 in inserted source. Re-exported from pristine Git bytes with explicit UTF-8; all 16 files reconstruct and hash correctly. No parser error was suppressed.
2. `research/engine-android-build-v2.log`: wrapper exit 1 / make exit 2 at the actual Model2 driver. A broad token rename changed native VC2 ROM/input/config identities and conflicted with its unchanged header declaration. A red test reproduced all 3 identity losses. The fixed exporter only changes host namespaces/includes/environment and the explicit HOTD capture selector, preserving every native ROM/input/machine symbol. Exactly 1 private engine file changed; all 16 patch states verify. The driver object then actually compiled.
3. `research/engine-android-build-v3.log`, former tool session29598: parent subsequently independently refreshed and confirmed the full ARM64 engine **exit 0**, recorded in parent `batch/hotd-android-engine-final.log`. The first full Windows engine also completed **exit 0**, reported by parent. The old29-header identity record in `docs/build-source-identity.json` is historical ABI1 identity, superseded by review repairs.

Parent refreshed all top-level native headers including new `start_policy.h` and successfully rebuilt both engines/hosts for ABI `0x484f5402`; `presentation.cpp` includes the corrected P2 help labels. Expected source header hashes are in `docs/review-header-manifest.json`. Logs in the parent batch folder are `hotd-review-windows-engine.log`, `hotd-review-android-engine.log`, `hotd-review-tests.log` and `hotd-review-android-host-build.log`, all exit 0. No private engine source/export was overwritten with an old snapshot. `tools/build-quest.ps1 -CheckOnly` enumerates real dependencies and does not substitute an engine.

Quest assembly used the installed SDK/JDK and a new isolated `D:/VR4ALL-HOTD-Model2-Build/gradle-user-home`, with build output and temporary files on D:. The pinned Gradle/AGP dependencies resolved online; no sibling build cache was copied. `tools/build-quest.ps1` completed exit 0 (`hotd-quest-build.log`). The APK contains all five required ARM64 libraries, package `com.vrlightgunhub.hotdmodel2`, versionCode 1 / `0.1.0-dev`, and a verified v2 signature. Preserve the new private signing key; certificate SHA256 is `b53d9e74da5c2327d9fb358a6af759391318c4df49bb876be56176c600e8ab07`. This is not a headset installation or launch.

## Review repairs and native coin evidence

Original coin code `0x1b6c8..0x1b76c` uses the operator coinage/bonus fields `0x51e194`/`0x51e1a0`; shared conversion `0x1b954..0x1b9d4` uses the derived denominator `0x51e192`. Accepted physical slot counts are separate backup-SRAM halfwords `0x01d0000e`/`0x01d00010`. Credit remainder is at `0x51ee40`/`0x51ee44`, fraction at `0x51e1ac`/`0x51e1ae`. Extra original-ROM fingerprints gate these reads. The reader does not assume one physical coin equals one credit.

Two real 30-second native runs used `tools/coin_timing_probe.lua`, the unchanged read-only reference executable/media and fresh private state. Both exit 0, no Lua error. `research/coin-timing-default-v1` accepted all six physical coins at paired 0.10 on/0.10 off, 0.15 on/0.15 off and 0.20 on/0.20 off timing; credits 1..6. `research/coin-timing-two-coins-v1` changed only the source-proven derived denominator to 2 in a disposable emulation process, not original media/user NVRAM. All six physical coins were accepted while balances were 0,1,1,2,2,3 and remainders 1,0,1,0,1,0. Exact CSV rows, argv, hashes and exits are in `docs/review-fixes.json`. This is native software timing evidence, not headset performance acceptance.

`NativeStartPolicy` retains the request's logical player and physical hand through a quick tap. It sends 0.15-second pulses with 0.15-second release, and another pulse requires the native accepted-slot counter to advance exactly one plus changed accounting balance/remainder/fraction. A reset/decrease/ambiguous wrap is not acknowledgement. Missing acknowledgement cancels without a retry coin train. Start waits until actual native selected-pool balance meets the current native price, or free play/already-live state requires no funding. Bounds are 32 coins, 20 seconds per request and 2 seconds per coin acknowledgement. Shared-pool requests serialize until earlier native debit/join acknowledgement; an earlier held Start retains its owner. Repricing, focus/menu/tracking loss, unknown state and deadline cancellation are covered by executable production tests.

P2-only Continue now uses native P2's own Start. Dual-to-P2-only retains P2's existing hand in either layout; held triggers require release before resuming. A simultaneous sole-hand transfer and fresh Start resolve ownership first, and controller help uses the same mapping. The setup tile now says HOTD.

Windows packaging follows actual imports from engine/host/loader directories plus MinGW, resolves transitive non-system DLLs and rejects missing dependencies. Required host exports are `hotd_model2_xr_start`/`hotd_model2_xr_stop`; loader requires `xrGetInstanceProcAddr`. ZIP validation rechecks closure against bundled AMD64 components only. Five tests compile and inspect real tiny PE/DLL fixtures, covering sibling/transitive DLL resolution, unresolved imports, wrong/missing exports and final ZIP rejection. This verifies packaging behavior; final HOTD candidate validation remains parent-owned.

## Actual native geometry and frame-clear repair

Parent's first 60-second packaged native simulation exited 0 with both native players live and acknowledged coins 2/2, but its world was black while HUD rendered. This is preserved in `research/portable-live-review-v1/result.json`, log and GL images. Normal VR capture intentionally skips CPU polygon rasterization, so its native screenshot also being black did not identify the failing stage.

Parent then captured complete original HOTD scenes with the same own engine, VR disabled and CPU reference raster enabled: `research/actual-scene-reference-v1`, exit 0. Replaying frame1439 through **unchanged** production GLScene showed the correct building, sky and character in both native and immersive views. Native projection RGB MAE was 0.031171, with 189444/190464 exact pixels. Setting depth to zero and omitting clear made the same world entirely black. Inspection confirmed `simulation()` never cleared its depth buffer; the actual OpenXR eye path already cleared every eye. No Model2C capture, material or lens patch was justified.

New `actual_hotd_host_geometry` loads the real production host DLL, renders a nearer raster/service frame, then actual captured gameplay frame2303. The old DLL fails deterministically: only 1295 lit pixels outside the original foreground mask, CTest exit 8 (`research/geometry-debug-v1/production-host-red.log`). Shared `begin_scene_frame()` in `native/gl_frame.h` now disables scissor, restores color/depth write masks, sets clear depth 1 and clears color/depth at each simulation frame and the existing real XR eye clear site. XR background clear color is preserved. **No existing native header, shared ABI, capture/material code or engine source changed.** The repaired real host passes with 160862 lit world pixels.

New `actual_hotd_scene_geometry` replays the same 2635-polygon actual gameplay frame through production GLScene. CPU reference RGB MAE 0.0734557 and 99.035% exact pixels pass the fidelity gate. A poisoned depth buffer, disabled write masks and 1-pixel scissor yield byte-identical corrected output. Immersive neutral/left/right/turned views contain 148841/146662/150416/156580 lit pixels; independent eyes differ in 127649 pixels and a 25-degree head turn changes 170498 pixels. Actual host, left-eye and turned images were visually inspected and contain the world. Logs and images are private under `research/geometry-debug-v1`; hashes and exact commands are recorded in `docs/render-repair.json`.

The Windows host rebuilt exit 0, SHA256 `20b6558766b9d3f6ab0ecc260e1687a84a8d1054d9384965b93bc512024a7a29`. Parent independently refreshed the ARM64 host and signed APK, then packaged both targets with dependency/export validation. Existing engines remain ABI2-compatible; new `gl_frame.h` is host-only. These software tests do not establish physical OpenXR/headset acceptance.

The current full verification log is `research/geometry-debug-v1/full-tests.log`, exit 0: **33 Python/Java/native/real-PE cases and 14/14 CTest**. Earlier counts remain initial/policy-repair history.

Parent independently repeated the final 33-case/14-CTest workflow (`hotd-render-fix-tests.log`), rebuilt the ARM64 host and assembled/signed the APK (`hotd-render-fix-android-host.log`, `hotd-render-fix-quest.log`), all exit 0. Gradle/AGP dependencies resolved in an isolated `D:/VR4ALL-HOTD-Model2-Build/gradle-user-home`; C: was not used for compiler or project caches. The independent local signing certificate SHA256 is `b53d9e74da5c2327d9fb358a6af759391318c4df49bb876be56176c600e8ab07`; preserve that private key for future app updates.

The reproducible portable check is `overseer/changes/2026-10-29-model2-last-three/verify_hotd_portable.py`, run with bundled Codex Python/Pillow. It uses only packaged runtime DLLs, removes compiler folders from PATH, verifies separate owned `hotdo` media and runs synthetic controllers with an observation-only Lua script. There is no Lua coin/Start/trigger injection. The repaired run (`research/portable-live-review-v3`) exits 0 after 60 native seconds, records both native live player states 5/5, 61 state samples, 28 actual GL images and up to **169,613 lit gameplay pixels**. Full CPU raster forcing is disabled, so visible gameplay comes from replayed geometry. The engine and host hashes match the packaged binaries. Sound output is disabled, no headset session exists, and native speed here is not a Quest/physical performance claim. An earlier harness attempt stopped before emulator launch due to Windows environment-key casing; it was corrected and rerun in a new private folder.

Final hashes/build identities are in `final-build-status.json` and the candidate directory. All exact parent commands and logs are in the batch report. A later documentation-only repack may change the ZIP hash; runtime engine/host hashes bind the tested executable behavior.

## Remaining gaps and release readiness

Native adapter gates are ROM fingerprints, actual native player acknowledgement and measured state; no trigger edge or handedness is treated as an accepted native shot. Common UI/persistence/fresh-input guards are implemented. Native pricing is bounded 0..5, respects current shared/per-player credit balance/free play, emits distinct bounded pulses and retains logical ownership. Corrupt prices/player identities and unknown ROMs fail closed.

Original gun extraction, HOTD HUD semantic separation, widened native camera/game zoom and complete-route/player transition coverage are unsupported or unverified. The native feedback continuation enables recoil/haptics from committed shots; physical output still needs headset validation. P2 Gun Adjustment handler is code-verified; its dynamic sampling and complete calibration/aim alignment remain untested. Service menu/aim phases are separate and exit is native-software verified.

No Quest/PCVR physical rendering, headset launch/play, actual audio/haptics, performance acceptance, downloaded release, installer discovery or publication has been tested. No headset or GitHub write operation was performed. Upstream HOTD remains marked `MACHINE_NOT_WORKING`; the diagnosed simulation depth fault was in this host and is not attributed to upstream. **Release readiness: false.** Continue complete calibration/aim/audio, route coverage and device acceptance work before considering any GitHub batch approval.

Exact selected common source hashes are in `docs/common-source-provenance.json`; engine file hashes in `docs/engine-adaptation-provenance.json`; current public source hashes in `docs/source-manifest.json`; actual host/patch artifact hashes in `docs/artifacts.json`; program-state evidence in `docs/native-evidence.json`. No media, extracted original meshes, private captures or signing key is included in source.
