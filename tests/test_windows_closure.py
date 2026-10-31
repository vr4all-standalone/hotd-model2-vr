"""Build real tiny AMD64 PE binaries and inspect their actual import/export tables."""
import importlib.util,os,shutil,subprocess,tempfile,unittest,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
READOBJ=Path('E:/AndroidSdk/ndk/29.0.14206865/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-readobj.exe')
GCC=Path('D:/msys64/mingw64/bin/gcc.exe')

class WindowsClosureTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        spec=importlib.util.spec_from_file_location('hotd_package',ROOT/'tools/package.py');cls.module=importlib.util.module_from_spec(spec);spec.loader.exec_module(cls.module)
        if not GCC.is_file()or not READOBJ.is_file():raise unittest.SkipTest('Actual MinGW AMD64 compiler/llvm-readobj required')
        cls.base=ROOT/'build/package-tests';cls.base.mkdir(parents=True,exist_ok=True)
        cls.fixture=tempfile.TemporaryDirectory(prefix='pe-fixture-',dir=cls.base);cls.fixture_root=Path(cls.fixture.name).resolve()
        env=dict(os.environ);env['PATH']=str(GCC.parent)+os.pathsep+env.get('PATH','');env['TEMP']=env['TMP']=str(cls.fixture_root)
        def compile(name,code,libraries=(),exe=False):
            source=cls.fixture_root/(name+'.c');source.write_text(code,encoding='utf8');out=cls.fixture_root/name
            args=[str(GCC),'-O0','-s','-static-libgcc',str(source),'-o',str(out),'-L',str(cls.fixture_root)]
            if not exe:args+=['-shared','-Wl,--out-implib,'+str(cls.fixture_root/('lib'+Path(name).stem+'.a'))]
            args+=['-l'+x for x in libraries]
            p=subprocess.run(args,capture_output=True,text=True,env=env);assert p.returncode==0,p.stdout+p.stderr
        compile('deep.dll','__declspec(dllexport) int deep(void){return 7;}')
        compile('codec.dll','__declspec(dllimport) int deep(void); __declspec(dllexport) int codec(void){return deep();}',('deep',))
        compile('hosthelper.dll','__declspec(dllexport) int hosthelper(void){return 3;}')
        compile('hotd_model2_xr.dll','__declspec(dllimport) int hosthelper(void); __declspec(dllexport) int hotd_model2_xr_start(void){return hosthelper();} __declspec(dllexport) void hotd_model2_xr_stop(void){}',('hosthelper',))
        compile('openxr_loader.dll','__declspec(dllexport) int xrGetInstanceProcAddr(void){return 0;}')
        compile('wrong_exports.dll','__declspec(dllexport) int unrelated_export(void){return 0;}')
        compile('hotd_model2.exe','__declspec(dllimport) int codec(void);int main(void){return codec();}',('codec',),True)
    @classmethod
    def tearDownClass(cls):
        if hasattr(cls,'fixture'):cls.fixture.cleanup()
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(prefix='pe-case-',dir=self.base);self.root=Path(self.temp.name).resolve()
        for d in ['engine','host','loader','mingw']:(self.root/d).mkdir()
        for name,folder in [('hotd_model2.exe','engine'),('codec.dll','engine'),('deep.dll','engine'),('hotd_model2_xr.dll','host'),('hosthelper.dll','host'),('openxr_loader.dll','loader')]:shutil.copy2(self.fixture_root/name,self.root/folder/name)
        self.engine=self.root/'engine/hotd_model2.exe';self.host=self.root/'host/hotd_model2_xr.dll';self.loader=self.root/'loader/openxr_loader.dll'
    def tearDown(self):self.temp.cleanup()
    def components(self):return self.module.windows_components(self.engine,self.host,self.loader,self.root/'mingw',READOBJ)
    def archive(self,exclude=()):
        path=self.root/'candidate.zip'
        with zipfile.ZipFile(path,'w')as z:
            for directory in ['engine','host','loader']:
                for p in (self.root/directory).glob('*'):
                    if p.name not in exclude:z.write(p,'hotd-model2-vr-pcvr/'+p.name)
        return path
    def test_sibling_and_transitive_dependencies_resolved_from_real_component_dirs(self):
        c=self.components();self.assertEqual(c['codec.dll'].resolve(),(self.root/'engine/codec.dll').resolve())
        self.assertEqual(c['deep.dll'].resolve(),(self.root/'engine/deep.dll').resolve())
        self.assertEqual(c['hosthelper.dll'].resolve(),(self.root/'host/hosthelper.dll').resolve())
    def test_transitive_missing_non_system_import_rejected(self):
        (self.root/'engine/deep.dll').unlink()
        with self.assertRaisesRegex(ValueError,'[Dd]eep.dll'):self.components()
    def test_wrong_host_exports_rejected(self):
        shutil.copy2(self.fixture_root/'wrong_exports.dll',self.host)
        with self.assertRaisesRegex(ValueError,'hotd_model2_xr_'):self.components()
    def test_missing_openxr_entrypoint_rejected(self):
        shutil.copy2(self.fixture_root/'wrong_exports.dll',self.loader)
        with self.assertRaisesRegex(ValueError,'xrGetInstanceProcAddr'):self.components()
    def test_zip_check_rechecks_actual_dependency_closure_and_exports(self):
        report=self.module.check_windows(self.archive(),READOBJ)
        self.assertTrue(report['dependency_closure_verified']and report['xr_exports_verified'])
        with self.assertRaisesRegex(ValueError,'[Dd]eep.dll'):self.module.check_windows(self.archive(['deep.dll']),READOBJ)
        shutil.copy2(self.fixture_root/'wrong_exports.dll',self.host)
        with self.assertRaisesRegex(ValueError,'hotd_model2_xr_'):self.module.check_windows(self.archive(),READOBJ)
if __name__=='__main__':unittest.main()
