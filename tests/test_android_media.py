import hashlib,os,tempfile,unittest,zipfile,warnings,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
TEMP_ROOT=ROOT/'research/android-tests'
TEMP_ROOT.mkdir(parents=True,exist_ok=True)
JDK=Path(os.environ.get('JAVA_HOME','E:/.gradle/jdks/eclipse_adoptium-21-amd64-windows.2'))
JAVA=ROOT/'android/app/src/main/java/com/vrlightgunhub/hotdmodel2/GameFiles.java'
class JavaMediaTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not (JDK/'bin/javac.exe').is_file(): raise unittest.SkipTest('Set JAVA_HOME to JDK 21 for executable media validation tests')
        if not JAVA.is_file(): return
        cls.temp = tempfile.TemporaryDirectory(prefix='hotdmodel2-java-', dir=TEMP_ROOT)
        cls.classes = Path(cls.temp.name)/'classes'; cls.classes.mkdir()
        subprocess.run([str(JDK/'bin/javac.exe'), '-d', str(cls.classes), str(JAVA), str(ROOT/'tests/java/com/vrlightgunhub/hotdmodel2/GameFilesHarness.java')], check=True, capture_output=True, text=True)

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, 'temp'): cls.temp.cleanup()

    def setUp(self):
        self.assertTrue(JAVA.is_file(), 'Android ROM validator has not been implemented')
        self.work = tempfile.TemporaryDirectory(dir=TEMP_ROOT); self.folder = Path(self.work.name)

    def tearDown(self):
        if hasattr(self, 'work'): self.work.cleanup()

    def archive(self, name, entries):
        path = self.folder/name
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
                for key, value in entries: z.writestr(key, value)
        return path

    def run_validation(self, paths, real=False):
        return subprocess.run([str(JDK/'bin/java.exe'), '-cp', str(self.classes), 'com.vrlightgunhub.hotdmodel2.GameFilesHarness', 'real' if real else 'synthetic', *map(str, paths)], text=True, capture_output=True)

    def test_split_and_complete_archives(self):
        game = self.archive('hotdmodel2.zip', [('game.rom', b'abc')]); board = self.archive('segabill.zip', [('board.rom', b'abc')])
        self.assertEqual(self.run_validation([game, board]).returncode, 0)
        game = self.archive('hotdmodel2.zip', [('game.rom', b'abc'), ('board.rom', b'abc')])
        self.assertEqual(self.run_validation([game]).returncode, 0)

    def test_missing_corrupt_duplicate_unsafe_and_oversized(self):
        cases = [([('game.rom', b'abc')], 'Missing'), ([('game.rom', b'xyz'), ('board.rom', b'abc')], 'checksum'), ([('game.rom', b'abc'), ('GAME.ROM', b'abc')], 'Duplicate'), ([('../game.rom', b'abc')], 'Unsafe'), ([('game.rom', b'abc'*100)], 'size')]
        for entries, message in cases:
            with self.subTest(message=message):
                result = self.run_validation([self.archive('hotdmodel2.zip', entries)])
                self.assertNotEqual(result.returncode, 0); self.assertIn(message.lower(), result.stderr.lower())

    def test_encrypted_and_truncated_archives(self):
        path = self.archive('hotdmodel2.zip', [('game.rom', b'abc'), ('board.rom', b'abc')]); data = bytearray(path.read_bytes())
        for signature, offset in [(b'PK\x03\x04', 6), (b'PK\x01\x02', 8)]:
            start = 0
            while (start := data.find(signature, start)) >= 0:
                data[start+offset] |= 1; start += 4
        path.write_bytes(data); result = self.run_validation([path]); self.assertNotEqual(result.returncode, 0)
        path.write_bytes(data[:20]); self.assertNotEqual(self.run_validation([path]).returncode, 0)

    def test_real_combined_media_when_available(self):
        path = ROOT/'private/roms/hotdo.zip'
        if not path.is_file(): self.skipTest('Private media not available')
        before = hashlib.sha256(path.read_bytes()).digest()
        result = self.run_validation([path], real=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(hashlib.sha256(path.read_bytes()).digest(), before)



if __name__=='__main__':unittest.main()
