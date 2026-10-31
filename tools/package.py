"""Create and validate local software-only The House of the Dead Model 2 VR deliveries."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import struct
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
REQUIRED_ANDROID_LIBS = {'libmain.so', 'libSDL2.so', 'libc++_shared.so', 'libhotd_model2_xr.so', 'libopenxr_loader.so'}
REQUIRED_WINDOWS = {'hotd_model2.exe', 'hotd_model2_xr.dll', 'openxr_loader.dll'}
VERSION = '0.1.7-beta'
DEFAULT_READOBJ = Path('E:/AndroidSdk/ndk/29.0.14206865/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-readobj.exe')
# Selected common portability rules from the sibling RC/BEL packaging review.
# These are Windows OS components; all other imports must be bundled/resolved.
WINDOWS_SYSTEM_DLLS = {
    'advapi32.dll','avrt.dll','bcrypt.dll','cfgmgr32.dll','comctl32.dll','comdlg32.dll','crypt32.dll',
    'd3d9.dll','d3d11.dll','d3d12.dll','d3dcompiler_47.dll','dbghelp.dll','dinput8.dll','dsound.dll',
    'dwmapi.dll','dxgi.dll','gdi32.dll','hid.dll','imm32.dll','iphlpapi.dll','kernel32.dll','kernelbase.dll',
    'msvcrt.dll','ntdll.dll','ole32.dll','oleaut32.dll','opengl32.dll','powrprof.dll','psapi.dll',
    'rpcrt4.dll','secur32.dll','setupapi.dll','shcore.dll','shell32.dll','shlwapi.dll','ucrtbase.dll',
    'user32.dll','userenv.dll','usp10.dll','version.dll','winhttp.dll','winmm.dll','wintrust.dll',
    'ws2_32.dll','wsock32.dll',
}


def check_names(names):
    """Reject traversal and media/state/keys before writing a public artifact."""
    rom_names = {x['name'].casefold() for x in json.loads((ROOT/'game-set.json').read_text())['files']}
    seen = set()
    for name in names:
        normalized = name.replace('\\', '/')
        path = PurePosixPath(normalized)
        parts = {part.casefold() for part in path.parts}
        key = normalized.casefold()
        if key in seen:
            raise ValueError(f'Duplicate package member: {name}')
        seen.add(key)
        private = {'private', 'research', 'userdata', 'nvram', 'cfg', 'state', 'snap', '.git'}
        if (not normalized or path.is_absolute() or ':' in normalized or '..' in path.parts
                or parts & private or path.name.casefold() in rom_names
                or path.name.casefold() in {'settings.ini', 'hotdo.zip', 'hotd.zip', 'custom.glb'}
                or path.suffix.casefold() in {'.zip', '.chd', '.nv', '.dif', '.jks', '.keystore', '.pem', '.key', '.log'}
                or (path.name.casefold().startswith('original') and path.suffix.casefold() in {'.glb', '.gltf', '.png', '.bin'})):
            raise ValueError(f'Private/game data cannot be packaged: {name}')


def _elf_arm64(header, name):
    if len(header) < 64 or header[:7] != b'\x7fELF\x02\x01\x01' or struct.unpack_from('<H', header, 18)[0] != 183:
        raise ValueError(f'Wrong native ABI: {name}; ARM64 ELF is required')


def check_apk(path):
    """Structural/media check; callers must separately verify the APK signature."""
    with zipfile.ZipFile(path) as archive:
        check_names(archive.namelist())
        names = set(archive.namelist())
        required = {'lib/arm64-v8a/'+name for name in REQUIRED_ANDROID_LIBS}
        if not required <= names:
            raise ValueError(f'Missing ARM64 libraries: {sorted(required-names)}')
        if 'AndroidManifest.xml' not in names:
            raise ValueError('APK has no Android manifest')
        for name in required:
            with archive.open(name) as stream:
                _elf_arm64(stream.read(64), name)
        for name in names:
            if name.startswith('lib/') and not name.startswith('lib/arm64-v8a/'):
                raise ValueError(f'Unexpected native ABI: {name}')
        font = archive.getinfo('assets/fonts/Roboto-Bold.ttf')
        with archive.open(font) as stream:
            if not 20000 <= font.file_size <= 5*1024*1024 or stream.read(4) != b'\0\1\0\0':
                raise ValueError('Menu font is not a complete TrueType font')
        damaged = archive.testzip()
        if damaged:
            raise ValueError(f'Corrupt APK member: {damaged}')
    return {'native_abi': 'arm64-v8a', 'libraries': sorted(REQUIRED_ANDROID_LIBS), 'game_data_included': False, 'signature_verified': False}


def verify_signature(path, apksigner):
    result = subprocess.run([str(apksigner), 'verify', '--verbose', '--print-certs', str(path)], check=True, capture_output=True, text=True)
    return {'signature_verified': True, 'signature_report': result.stdout.strip()}


def _pe_amd64(stream, name):
    header = stream.read(64)
    if len(header) != 64 or header[:2] != b'MZ':
        raise ValueError(f'Not a Windows executable: {name}')
    offset = struct.unpack_from('<I', header, 60)[0]
    if offset < 64 or offset > 1024*1024:
        raise ValueError(f'Invalid PE header: {name}')
    stream.read(offset-64)
    signature = stream.read(6)
    if len(signature) != 6 or signature[:4] != b'PE\0\0' or struct.unpack_from('<H', signature, 4)[0] != 0x8664:
        raise ValueError(f'Wrong Windows ABI: {name}; AMD64 is required')


def _readobj(tool, option, path):
    tool=Path(tool or DEFAULT_READOBJ)
    if not tool.is_file():raise FileNotFoundError('llvm-readobj is required to verify actual PE imports/exports: '+str(tool))
    return subprocess.run([str(tool),option,str(path)],check=True,text=True,capture_output=True,timeout=30).stdout


def windows_components(engine,host,loader,mingw,readobj):
    """Resolve real AMD64 imports from component locations, including dynamic XR.

    Passing mingw=None verifies a package using bundled components only.
    Native exports are inspected before any candidate is called verified.
    """
    engine,host,loader=map(Path,(engine,host,loader))
    roots=[engine.parent,host.parent,loader.parent]+([Path(mingw)]if mingw is not None else[])
    queue=[(engine,engine.name),(host,host.name),(loader,'openxr_loader.dll')]
    pending={name.casefold()for _,name in queue};components={}
    for source,target_name in queue:
        with source.open('rb')as stream:_pe_amd64(stream,source.name)
        components[target_name]=source
        text=_readobj(readobj,'--coff-imports',source)
        for name in re.findall(r'^\s*Name: (.+\.dll)\s*$',text,re.MULTILINE|re.IGNORECASE):
            name=name.strip();key=name.casefold()
            if '/'in name or '\\'in name or ':'in name or any(ord(c)<32 for c in name):raise ValueError('Unsafe DLL import: '+name)
            if key in pending or key in WINDOWS_SYSTEM_DLLS or key.startswith(('api-ms-win-','ext-ms-win-')):continue
            dependency=None
            for directory in [source.parent,*roots]:
                candidate=directory/name
                if candidate.is_file():dependency=candidate;break
                if directory.is_dir():
                    dependency=next((p for p in directory.iterdir()if p.is_file()and p.name.casefold()==key),None)
                    if dependency is not None:break
            if dependency is None:raise ValueError(f'Unresolved Windows dependency {name} imported by {source.name}')
            pending.add(key);queue.append((dependency,name))
    for source,required in [(host,{'hotd_model2_xr_start','hotd_model2_xr_stop'}),(loader,{'xrGetInstanceProcAddr'})]:
        text=_readobj(readobj,'--coff-exports',source)
        exports={s.strip()for s in re.findall(r'^\s*Name: (.+)$',text,re.MULTILINE)}
        if not required<=exports:raise ValueError(f'Missing native export in {source.name}: {sorted(required-exports)}')
    return components


def check_windows(path,readobj=None):
    with zipfile.ZipFile(path) as archive:
        check_names(archive.namelist())
        names = {PurePosixPath(name).name.casefold() for name in archive.namelist()}
        if not REQUIRED_WINDOWS <= names:
            raise ValueError(f'Missing Windows components: {sorted(REQUIRED_WINDOWS-names)}')
        binaries={}
        for name in archive.namelist():
            if PurePosixPath(name).suffix.casefold() in {'.exe', '.dll'}:
                with archive.open(name) as stream: _pe_amd64(stream, name)
                key=PurePosixPath(name).name.casefold()
                if key in binaries:raise ValueError('Ambiguous native DLL basename: '+key)
                binaries[key]=name
        directory=PurePosixPath(binaries['hotd_model2.exe']).parent
        if any(PurePosixPath(name).parent!=directory for name in binaries.values()):raise ValueError('Portable native binaries must share the engine directory')
        damaged = archive.testzip()
        if damaged: raise ValueError(f'Corrupt Windows package: {damaged}')
        temporary_root=ROOT/'build/package-verification';temporary_root.mkdir(parents=True,exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='hotd-pe-',dir=temporary_root)as temp:
            root=Path(temp)
            for key,name in binaries.items():
                with archive.open(name)as source,(root/key).open('wb')as target:shutil.copyfileobj(source,target)
            components=windows_components(root/'hotd_model2.exe',root/'hotd_model2_xr.dll',root/'openxr_loader.dll',None,readobj)
    return {'native_abi': 'x86_64', 'game_data_included': False,'dependency_closure_verified':True,'xr_exports_verified':True,'native_components':sorted(components)}


def windows(engine, host, loader, mingw, readobj, output):
    output.mkdir(parents=True, exist_ok=True)
    (ROOT/'build').mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='hotd-package-', dir=ROOT/'build') as temporary:
        folder = Path(temporary)/'hotd-model2-vr-pcvr'; folder.mkdir()
        components=windows_components(engine,host,loader,mingw,readobj)
        for target_name,source in components.items():shutil.copy2(source,folder/target_name)
        for source in ['tools/launch-pcvr.ps1', 'tools/run-pcvr.cmd', 'CREDITS.md', 'LICENSE', 'engine-lock.json', 'game-set.json', 'README.md', 'docs/validation.md']:
            target = folder/(Path(source).name if source.startswith('tools/') else source)
            target.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(ROOT/source, target)
        for directory in ['assets', 'licenses']:
            for source in (ROOT/directory).rglob('*'):
                if source.is_file():
                    relative = source.relative_to(ROOT); check_names([relative.as_posix()])
                    target = folder/relative; target.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(source, target)
        (folder/'README.txt').write_text('''The House of the Dead Model 2 VR — local development build

Enable your PC OpenXR runtime and connect the headset. Run run-pcvr.cmd.
Place owned original arcade files in roms/hotdo.zip,
A verified old-emulator hotd.zip can also supply the original-revision set.
To use another ROM folder: launch-pcvr.ps1 -RomDirectory D:\\MyRoms
Keep ROM archives packed. NVRAM, settings and saved state use userdata/.
No game data is included. See CREDITS.md, licenses/ and docs/validation.md
for implementation and physical-headset validation status.
''', encoding='utf-8')
        (folder/'build-info.json').write_text(json.dumps({'engine': json.loads((ROOT/'engine-lock.json').read_text()), 'development_build': True, 'game_data_included': False}, indent=2), encoding='utf-8')
        files = sorted(file for file in folder.rglob('*') if file.is_file())
        check_names([file.relative_to(folder).as_posix() for file in files])
        archive = output/'hotd-model2-vr-pcvr.zip'
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as zipout:
            for file in files: zipout.write(file, 'hotd-model2-vr-pcvr/'+file.relative_to(folder).as_posix())
    check_windows(archive,readobj)
    return archive


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check-apk', type=Path)
    parser.add_argument('--check-windows', type=Path)
    parser.add_argument('--apksigner', type=Path, default=Path('E:/AndroidSdk/build-tools/36.0.0/apksigner.bat'))
    parser.add_argument('--apk', type=Path, default=ROOT/'build/android/app/outputs/apk/release/app-release.apk')
    parser.add_argument('--windows-only', action='store_true', help='Package verified PCVR binaries without requiring a Quest APK')
    parser.add_argument('--engine', type=Path); parser.add_argument('--host', type=Path)
    parser.add_argument('--loader', type=Path); parser.add_argument('--mingw-bin', type=Path); parser.add_argument('--readobj', type=Path)
    parser.add_argument('--output', type=Path, default=ROOT/'build/delivery')
    args = parser.parse_args()
    if args.check_apk:
        report = check_apk(args.check_apk); report.update(verify_signature(args.check_apk, args.apksigner))
        print(json.dumps(report, indent=2)); return
    if args.check_windows: print(json.dumps(check_windows(args.check_windows,args.readobj), indent=2)); return
    args.output.mkdir(parents=True, exist_ok=True)
    files = []; report = {}
    if not args.windows_only:
        quest = check_apk(args.apk); quest.update(verify_signature(args.apk, args.apksigner))
        apk = args.output/'hotd-model2-vr-quest.apk'; shutil.copy2(args.apk, apk)
        files.append(apk); report['quest'] = quest
    if args.windows_only or any([args.engine, args.host, args.loader, args.mingw_bin, args.readobj]):
        if not all([args.engine, args.host, args.loader, args.mingw_bin, args.readobj]): parser.error('PC packaging requires --engine, --host, --loader, --mingw-bin and --readobj')
        archive = windows(args.engine, args.host, args.loader, args.mingw_bin, args.readobj, args.output)
        files.append(archive); report['windows'] = check_windows(archive,args.readobj)
    with (args.output/'SHA256SUMS.txt').open('w', encoding='utf-8') as out:
        for file in files:
            with file.open('rb') as incoming: digest = hashlib.file_digest(incoming, 'sha256').hexdigest()
            out.write(f'{digest}  {file.name}\n')
    (args.output/'artifact-validation.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps({'files': list(map(str, files)), 'validation': report}, indent=2))


if __name__ == '__main__': main()
