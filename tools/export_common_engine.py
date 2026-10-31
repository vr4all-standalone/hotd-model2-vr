"""Reproduce selected licensed common engine deltas; preserve native game IDs.

Optional maintainer tool: normal builds use the self-contained exported patch.
The reference is read-only; only the explicit private engine is written.
"""
import argparse,difflib,hashlib,json,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
COMMIT='f34f02505e32c1993c6a782b6814232cbfc74e36'

def export(reference,engine):
    reference=Path(reference).resolve();engine=Path(engine).resolve()
    if reference==engine:raise ValueError('Reference must remain read-only')
    manifest=json.loads((ROOT/'docs/engine-adaptation-provenance.json').read_text(encoding='utf8'))
    patch='';updates=[]
    for item in manifest['files']:
        rel=item['file'];raw=(reference/rel).read_bytes()
        if hashlib.sha256(raw).hexdigest()!=item['reference_sha256']:raise ValueError('Reference changed: '+rel)
        old=subprocess.check_output(['git','-C',str(reference),'show',COMMIT+':'+rel]).decode('utf8').replace('\r\n','\n')
        s=raw.decode('utf8').replace('\r\n','\n')
        if rel=='src/mame/sega/model2.cpp':
            s,n=re.subn(r'    if\(vcop2::vr_requested\(\) && std::string_view\(machine\(\).system\(\).name\)=="vcop2"\) \{.*?\n    \}\n','',s,count=1,flags=re.S)
            if n!=1:raise ValueError('Expected one game-specific camera-tap block')
            anchor='    io.in_pc_callback().set([this](){return vcop2::trigger_input(ioport("IN1")->read());});'
            if s.count(anchor)!=1:raise ValueError('Expected one Model2A trigger hook')
            s=s.replace(anchor,'')
            anchor='\tio.serial_ch2_wr_callback().set(FUNC(model2c_state::lightgun_mux_w));'
            if s.count(anchor)!=1:raise ValueError('Expected one HOTD Model2C serial hook')
            s=s.replace(anchor,anchor+'\n    io.in_pc_callback().set([this](){return vcop2::trigger_input(ioport("IN1")->read());});')
        if rel=='src/mame/sega/model2_v.cpp':
            s=s.replace('#include "vcop2/native_flash.h"\n','')
            start=s.index('    // Hide only the verified original gun-reader');end=s.index('\t// if the scroll color table',start)
            s=s[:start]+'    const uint16_t* window_mask_override=nullptr; // No HOTD native mask override verified.\n'+s[end:]
        # Host namespace/include/environment identities ONLY. Native vcop2 ROM,
        # INPUT_PORTS/GAME and model2a_state::vcop2 configuration must survive.
        s=s.replace('VCOP2','HOTD_MODEL2').replace('vcop2::','hotd_model2::').replace('"vcop2/','"hotd_model2/').replace('/lib/vcop2/','/lib/hotd_model2/')
        s=s.replace('machine().system().name) == "vcop2"','machine().system().name) == "hotdo"').replace('machine().system().name)=="vcop2"','machine().system().name)=="hotdo"')
        if rel=='src/mame/sega/model2.cpp':
            for pattern in [r'ROM_START\(\s*(\w+)',r'INPUT_PORTS_START\(\s*(\w+)',r'void (\w+_state::\w+)\(machine_config &config\)']:
                if set(re.findall(pattern,s))!=set(re.findall(pattern,old)):raise ValueError('Native game identity changed')
        if '0x506044' in s or '0x506140' in s:raise ValueError('Borrowed game camera tap')
        data=s.encode('utf8');item['result_sha256']=hashlib.sha256(data).hexdigest()
        patch+=''.join(difflib.unified_diff(old.splitlines(True),s.splitlines(True),fromfile='a/'+rel,tofile='b/'+rel))
        target=engine/rel
        if target.read_bytes().replace(b'\r\n',b'\n')!=data:updates.append((target,data))
    # All source checks run before any mutation.
    for target,data in updates:target.write_bytes(data)
    (ROOT/'patches/0001-native-hotd-host.patch').write_text(patch,encoding='utf8',newline='\n')
    (ROOT/'docs/engine-adaptation-provenance.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf8')
    print(f'Exported {len(manifest["files"])} selected deltas; updated {len(updates)} private engine files')
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--engine',type=Path,required=True)
    a=p.parse_args();export(a.reference,a.engine)
