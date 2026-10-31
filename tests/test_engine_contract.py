"""Check the applied real MAME driver preserves every native game identity."""
import re,subprocess,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ENGINE=(ROOT/'build').resolve().parent/'engine'
COMMIT='f34f02505e32c1993c6a782b6814232cbfc74e36'
class EngineContractTests(unittest.TestCase):
    def test_native_mame_game_and_machine_symbols_preserved(self):
        if not (ENGINE/'.git').is_dir():self.skipTest('Private pinned upstream engine not prepared')
        original=subprocess.check_output(['git','-C',str(ENGINE),'show',COMMIT+':src/mame/sega/model2.cpp']).decode('utf8')
        actual=(ENGINE/'src/mame/sega/model2.cpp').read_text(encoding='utf8')
        for pattern in [r'ROM_START\(\s*(\w+)',r'INPUT_PORTS_START\(\s*(\w+)',r'void (\w+_state::\w+)\(machine_config &config\)']:
            with self.subTest(pattern=pattern):self.assertEqual(set(re.findall(pattern,original)),set(re.findall(pattern,actual)))
        self.assertIn('FUNC(model2c_state::lightgun_mux_w)',actual)
        self.assertIn('hotd_model2::trigger_input(ioport("IN1")->read())',actual)
if __name__=='__main__':unittest.main()
