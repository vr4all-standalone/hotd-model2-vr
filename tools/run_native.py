"""Bounded headless original HOTD diagnostic using isolated writable state."""
import argparse,hashlib,json,os,subprocess,time
from pathlib import Path

def run(engine,roms,output,seconds=35,script=None):
    engine=Path(engine).resolve();roms=Path(roms).resolve();output=Path(output).resolve()
    if not 1<=seconds<=120:raise ValueError('Bounded native seconds must be 1..120')
    if output.exists():raise FileExistsError(output)
    if output.is_relative_to(roms) or roms.is_relative_to(output):raise ValueError('Keep state separate from media')
    if not (roms/'hotdo.zip').is_file():raise FileNotFoundError(roms/'hotdo.zip')
    output.mkdir(parents=True)
    args=[str(engine),'hotdo','-rompath',str(roms),'-skip_gameinfo','-noreadconfig','-video','none','-sound','none','-seconds_to_run',str(seconds),'-nothrottle','-noautoframeskip','-frameskip','0','-autoboot_script',str(Path(script).resolve() if script else Path(__file__).with_name('observe_native.lua').resolve()),'-autoboot_delay','0']
    for option,folder in [('nvram_directory','nvram'),('cfg_directory','cfg'),('diff_directory','diff'),('state_directory','state'),('snapshot_directory','snap')]:
        p=output/folder;p.mkdir();args.extend(['-'+option,str(p)])
    env=dict(os.environ);env['HOTD_MODEL2_RESEARCH_DIR']=str(output);env['TMP']=env['TEMP']=str(output)
    for key in list(env):
        if key.startswith(('VCOP2_','VCOP_','HOTD_MODEL2_VR','HOTD_MODEL2_XR')):env.pop(key)
    env['PATH']='D:/msys64/mingw64/bin'+os.pathsep+env.get('PATH','')
    start=time.monotonic()
    with (output/'engine.log').open('w') as log:
        p=subprocess.Popen(args,cwd=output,env=env,stdout=log,stderr=subprocess.STDOUT,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        try:code=p.wait(timeout=max(120,seconds*8))
        except subprocess.TimeoutExpired:
            p.terminate()
            try:p.wait(timeout=10)
            except subprocess.TimeoutExpired:p.kill();p.wait()
            code=-1
    text=(output/'engine.log').read_text(errors='replace')
    result=dict(command=args,engine_sha256=hashlib.sha256(engine.read_bytes()).hexdigest(),exit_code=code,requested_native_seconds=seconds,wall_seconds=time.monotonic()-start,lua_error='[LUA ERROR]' in text,headless=True,headset=False)
    (output/'run.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2));return code if code else int(result['lua_error'])

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--script');p.add_argument('--engine',required=True);p.add_argument('--roms',required=True);p.add_argument('--output',required=True);p.add_argument('--seconds',type=int,default=35)
    raise SystemExit(run(**vars(p.parse_args())))
