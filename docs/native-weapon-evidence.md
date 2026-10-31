# Original HOTD scene pistol — 2026-10-29

The original **hotdo** media contains a usable, textured pistol with an authored
right hand. The port now extracts model `0x719`, named `PN_ter00a`, from the owned
game data at runtime. This is an original scene/character pistol used cosmetically
as a tracked VR gun. A continuously rendered native first-person weapon and a
native equipment/pickup selector have not been verified. No VC1/VC2 gun, invented
geometry, ROM payload or extracted asset is included in public source.

## Native evidence

The private original program SHA256 is
`da2315b0b044d279728c8689336da6de0fee5997cf006514a1a633bd0de2fc75`.
The inspection tool verifies all 29 media payloads against `game-set.json` before
writing research output. It independently enumerates 7,465 named model records.

| Evidence | Original address/value |
| --- | --- |
| Model draw | `0x13fcc`: `lda 0x02e73530[g0*16],g0` |
| Record submission | `0x13fd4`: load four words; `0x13fd8`: store to `0x00804000` |
| Model catalogue | `0x02e73530`, 16-byte records: texture points, texture header, object, link count |
| Authored model name | `0x02e54560`: `PN_ter00a` |
| Model record | `0x02e7a6c0`: `0x9764e`, `0x977de`, `0x8e7f31`, `51` |
| Character skeleton references | `0xb47f0`, `0xb4aa0`, `0xbbf20`, `0xbd050`: model `0x719` |
| Extracted content | 51 native strip links, including one non-rendering setup; 50 rendered polygons |

The related `PN_ter00b` model `0x71b` has a retracted slide and explicit draw
references at `0x9d488`, `0x9d528`, `0x9d5d0`, `0x9d680` and `0x9d728`.
It is not substituted for the closed-slide pistol and no unverified reload
animation is driven from controller input. `PN_magazin` and
`PN_te_magazin_l00a` are separate original magazine/left-hand objects. The model
named `PN_hndl_buki_CT000a` is a spear-like prop, not the player pistol.

The decoder preserves native triangle/quad strip continuation, original normals,
UVs, texture headers and non-rendering links. A triangle's unused fourth ROM point
must be replaced by its third point before the following link; retaining it would
add a false origin vertex to this actual pistol. Fingerprints, the exact supported
record, bounded reads, finite coordinates and the exact terminator/count fail
closed on unsupported or malformed input. Only native weapon ID 1 is supported.

## Authored palm, muzzle and presentation

The first 31 rendered polygons are the complete original hand surface. After
matching duplicated authored vertices to five decimal places, every edge is
shared exactly twice: no cap or invented surface is needed. Signed tetrahedral
volume integration gives volume `0.24265748059004646` in source units cubed and
palm centroid:

`(-0.12873517341366428, -0.5152499676227664, -0.04314625931326567)`.

Rendered polygon 45 is the original front barrel cap. Its four-point centroid is:

`(-0.37503307685256004, -2.2581366300582886, 0.6543127968907356)`.

The authored barrel points along **-Y** and its top points along **+Z**. The
extractor retains source coordinates. The presentation maps source `(x,y,z)` to
controller `(-x,z,y)` after translating the measured palm to the origin. A uniform
`0.1` metres per source unit is a physical presentation choice, not a claim that
the ROM contains metre units. The resulting original pistol spans about 0.228 m
along the barrel axis before the user's gun-size adjustment. Physical comfort,
alignment and left-hand mirroring still require a headset check.

The common selector is original/Arcade/Zapper, with serialized original choice
8 and original mesh cache slot 8. It resolves the verified scene pistol ID 1
independently of any unverified native equipment field. `standard_fallback=true`
marks its cosmetic nature, and `follows_game=false` avoids claiming pickup
tracking. Existing Arcade choice 0 and Zapper choice 1 retain their identities.

## Focused verification

`tests/native_weapon_test.cpp` first failed at the real owned-ROM extraction
assertion while the production extractor was still its empty placeholder. The
implemented extractor then passed with `-Wall -Wextra -Werror` and assertions
enabled. The focused test checks the literal native model identity, 50 polygons,
authored coordinates/headers/UVs, triangle continuation, original muzzle cap,
unsupported weapon rejection, truncation, changed revision/name/record/skeleton
and NaN rejection, plus the selector's original/Arcade/Zapper cycle.

With the optional real gameplay capture argument, it also renders the extracted
pistol through the production `Model2Resources` sampler using the original
captured texture sheets, palette, color translation, luma and gamma. The checked
run painted **402,501 samples with 127 distinct colors**. Private PNG inspection
shows the authentic black/silver pistol, dark red grip and authored hand. This is
software material/geometry evidence; it is not headset or integrated host evidence.

Private evidence directory:
`D:/VR4ALL-HOTD-Model2-Build/research/native-weapon-v1/`.
It contains the catalogue, model previews, full private test fixtures,
`evidence.json`, `native-pistol.ppm` and the inspected `native-pistol.png`.

```powershell
& '<local-user>/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' -B tools/research_native_weapon.py --archive 'D:/VR4ALL-HOTD-Model2-Build/private/roms/hotdo.zip' --output 'D:/VR4ALL-HOTD-Model2-Build/research/native-weapon-v1' --fixtures
$env:PATH='D:/msys64/mingw64/bin;'+$env:PATH
& 'D:/msys64/mingw64/bin/g++.exe' -std=c++20 -O0 -Wall -Wextra -Werror -UNDEBUG tests/native_weapon_test.cpp -o 'D:/VR4ALL-HOTD-Model2-Build/research/native-weapon-v1/native_weapon_test.exe'
& 'D:/VR4ALL-HOTD-Model2-Build/research/native-weapon-v1/native_weapon_test.exe' 'D:/VR4ALL-HOTD-Model2-Build/research/native-weapon-v1' 'D:/VR4ALL-HOTD-Model2-Build/research/actual-scene-reference-v1/scenes/frame-00002303'
```

The three commands exited 0 after implementation. A preliminary compiler call
without the MinGW runtime directory in PATH exited 1 without compiler output;
setting PATH as shown resolved that environment issue. The focused truncation
test's limit was corrected to exclude the actual terminator at polygon word
`0xe8135`; the next model begins at word `0xe8136`.

Private region SHA256:

| Region | SHA256 |
| --- | --- |
| main data, including native ROM_COPY mirrors | `a6b137701a41e86beccc1ebb9780f22a27690229f5f9cc4f77b9b9c00c71fa74` |
| polygons | `fe06dee0af64a9bf56da4f14f8cd3029f5c6b562b375067733e9dc78bab0c191` |
| texture ROM | `3a3d8b413513a1c0ce74af07048dc8cb31d807e2a88d7dc5852da3acdf551402` |

## Host integration owned by the parent task

The extraction change alone does not complete the user-visible feature. The
parent task must update the following existing integration points and verify the
result before claiming it is available in a fresh build:

- `host_bridge.h`: extract/cache only the verified native weapon ID 1. Its copied
  VC2 loop over IDs 1..6 aborts when ID 2 correctly returns unavailable.
- `presentation.cpp`: original name/selection, measured palm/muzzle, correct
  source-axis rotation and 0.1 physical scale; preserve retry/fallback on missing
  game resources and the existing controller-space left-hand reflection.
- `gun_pose.h`: measured original palm and muzzle in the preserved source units.
- Settings: deliberate original default/migration while preserving explicit
  Arcade/Zapper user choices under the parent task's chosen policy.
- Test registration: run the focused test with its private fixture directory and
  optional real capture directory; keep every fixture out of source packages.
- Run the complete production Python/CTest suites and fresh supported host,
  Quest and PCVR builds. No engine build, headset operation, publication or claim
  of physical acceptance was performed by the extraction subtask.
