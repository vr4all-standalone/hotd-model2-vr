"""Bounded packaged HOTD shot-event proof using synthetic hands; no headset/audio."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import time
import zipfile

OBSERVER = r'''local root=assert(os.getenv('HOTD_MODEL2_RESEARCH_DIR'))
local m=manager.machine;local cpu=m.devices[':maincpu'];local mem=cpu.spaces.program
local out=assert(io.open(root..'/native-events.tsv','w'));local taps={}
out:write('time\tevent\tplayer\n')
local function live(p)
 return mem:read_u16(0x51ee14)==4 and mem:read_u32(0x51e258)==0 and mem:read_u32(0x51ec04+p*0xcc)==5 and mem:read_u32(0x51ec10+p*0xcc)>0
end
local function event(tag,p)
 out:write(string.format('%.6f\t%s\t%d\n',m.time:as_double(),tag,p+1));out:flush()
end
taps[#taps+1]=mem:install_write_tap(0x51ec00,0x51ed97,'hotd-independent-write-observer',function(a,d,mask)
 local ip=cpu.state['ip'].value
 for p=0,1 do
  if live(p) then
   if ip==0x5c40 and a==0x51ec08+p*0xcc and mask==0xffffffff and (d&2)~=0 then event('committed',p) end
   if a==0x51ec40+p*0xcc and mask==0xff00 then
    if ip==0x5cc0 then event('refunded',p) end
    if ip==0x5cec then event('reload',p) end
   end
  end
 end
end)
for p=0,1 do
 local player=p
 taps[#taps+1]=mem:install_read_tap(0x51ec40+p*0xcc,0x51ec43+p*0xcc,'hotd-independent-empty-observer',function(a,d,mask)
  if cpu.state['ip'].value==0x5bd0 and mask==0xff00 and (d&0xff00)==0 and live(player) then event('dry',player) end
 end)
end
hotd_independent_shot_taps=taps
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--roms', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out, roms = args.output.resolve(), args.roms.resolve()
    private = Path('D:/VR4ALL-HOTD-Model2-Build').resolve()
    if out.exists() or not out.is_relative_to(private/'research'):
        raise ValueError('Use a new HOTD private research folder on D:')
    if out.is_relative_to(roms) or roms.is_relative_to(out) or not (roms/'hotdo.zip').is_file():
        raise ValueError('Keep owned media outside the research output')
    out.mkdir(parents=True)
    with zipfile.ZipFile(args.archive) as archive:
        for name in archive.namelist():
            if Path(name).is_absolute() or '..' in Path(name).parts or ':' in name:
                raise ValueError(name)
        archive.extractall(out/'portable')
    portable = out/'portable/hotd-model2-vr-pcvr'
    script = out/'observe-only.lua'
    script.write_text(OBSERVER, encoding='utf-8')
    env = dict(os.environ)
    for key in list(env):
        if key.startswith(('HOTD_MODEL2_', 'VCOP2_', 'VCOP_', 'GUNBLADE_')):
            env.pop(key)
    system = Path(os.environ['SystemRoot'])
    env['PATH'] = os.pathsep.join((str(portable), str(system/'System32'), str(system)))
    env.update(TEMP=str(out), TMP=str(out), HOTD_MODEL2_VR='1', HOTD_MODEL2_XR_SIM='1',
               HOTD_MODEL2_SIM_GUN_TEST='1', HOTD_MODEL2_SIM_RELOAD_TEST='1',
               HOTD_MODEL2_XR_LIBRARY=str(portable/'hotd_model2_xr.dll'),
               HOTD_MODEL2_ASSETS=str(portable/'assets'), HOTD_MODEL2_SETTINGS=str(out/'settings.ini'),
               HOTD_MODEL2_XR_LOG=str(out/'xr.log'), HOTD_MODEL2_RESEARCH_DIR=str(out))
    command = [str(portable/'hotd_model2.exe'), 'hotdo', '-rompath', str(roms),
               '-skip_gameinfo', '-noreadconfig', '-video', 'none', '-sound', 'none',
               '-seconds_to_run', '90', '-throttle', '-speed', '3', '-noautoframeskip',
               '-frameskip', '0', '-autoboot_script', str(script), '-autoboot_delay', '0']
    for option, folder in [('nvram_directory', 'nvram'), ('cfg_directory', 'cfg'),
                           ('diff_directory', 'diff'), ('state_directory', 'state'), ('snapshot_directory', 'snap')]:
        (out/folder).mkdir()
        command += ['-'+option, str(out/folder)]
    started = time.monotonic()
    with (out/'engine.log').open('w', encoding='utf-8') as log:
        process = subprocess.Popen(command, cwd=out, env=env, stdout=log, stderr=subprocess.STDOUT,
                                   creationflags=subprocess.CREATE_NO_WINDOW)
        try:
            code = process.wait(timeout=180)
        except subprocess.TimeoutExpired:
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill();process.wait()
            code = -1
    engine_log = (out/'engine.log').read_text(encoding='utf-8', errors='replace')
    host_log = (out/'xr.log').read_text(encoding='utf-8', errors='replace') if (out/'xr.log').exists() else ''
    rows = (out/'native-events.tsv').read_text().splitlines()[1:]
    events = {tag: [0, 0] for tag in ('committed', 'refunded', 'reload', 'dry')}
    for row in rows:
        _, tag, player = row.split('\t')
        events[tag][int(player)-1] += 1
    final = re.findall(r'Diagnostic final accepted counts: (\d+)/(\d+)', host_log)
    totals = list(map(int, final[-1])) if final else None
    feedback = [[], []]
    for player, count in re.findall(r'Diagnostic accepted player (\d) shot (\d+)', host_log):
        feedback[int(player)-1].append(int(count))
    result = dict(exit_code=code, native_seconds=90, wall_seconds=round(time.monotonic()-started, 3),
                  independent_native_events=events, host_native_totals=totals, host_feedback_counts=feedback,
                  headset_used=False, audio_listened=False, physical_haptics_tested=False,
                  no_scripted_cabinet_inputs=True, only_packaged_runtime_dependencies=True,
                  package_sha256=hashlib.sha256(args.archive.read_bytes()).hexdigest(), command=command)
    errors = []
    if code != 0 or '[LUA ERROR]' in engine_log: errors.append('Native run failed')
    if totals != events['committed'] or not all(events['committed']): errors.append('Native totals differ or a player never fired')
    if not all(feedback): errors.append('Feedback did not reach both player paths')
    for p in range(2):
        if any(b <= a for a, b in zip(feedback[p], feedback[p][1:])): errors.append('Feedback repeated an accepted counter')
        if any(n > events['committed'][p] for n in feedback[p]): errors.append('Feedback invented a shot')
    if not sum(events['refunded']): errors.append('Blocked-shot refund was not exercised')
    if not sum(events['reload']): errors.append('Reload was not exercised')
    if not sum(events['dry']): errors.append('Empty-gun branch was not exercised')
    result['errors'] = errors
    (out/'result.json').write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(result, indent=2))
    if errors:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
