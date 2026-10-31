import importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class PackageTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        spec=importlib.util.spec_from_file_location('package',ROOT/'tools/package.py');cls.module=importlib.util.module_from_spec(spec);spec.loader.exec_module(cls.module)
    def test_private_game_and_key_paths_rejected(self):
        for path in ['private/sign.keystore','userdata/settings.ini','roms/hotdo.zip','roms/hotd.zip','epr-19696.15','../escape','C:/absolute','original-gun.glb']:
            with self.subTest(path=path),self.assertRaises(ValueError):self.module.check_names([path])
    def test_public_code_and_licensed_assets_allowed(self):self.module.check_names(['hotd_model2_xr.dll','assets/models/guns/zapper.glb','assets/fonts/Roboto-Bold.ttf','CREDITS.md'])
    def test_wrong_native_abi_rejected(self):
        with self.assertRaises(ValueError):self.module._elf_arm64(b'not elf','libmain.so')
if __name__=='__main__':unittest.main()
