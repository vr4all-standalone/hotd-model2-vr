"""Exercise the PCVR launcher's real PowerShell argument construction."""

import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[1]
LAUNCHER = Path(os.environ.get('HOTD_PC_LAUNCHER_UNDER_TEST', ROOT / 'tools/launch-pcvr.ps1'))


class PcvrLauncherArgumentsTests(unittest.TestCase):
    def test_data_directory_options_are_separate_arguments(self):
        powershell = shutil.which('powershell.exe')
        if not powershell:
            self.skipTest('Windows PowerShell is required to exercise the Windows launcher')

        source = LAUNCHER.read_text(encoding='utf-8-sig')
        match = re.search(
            r"(\$taskArgs=@\('hotdo'.*?)(?=\s*& \(Join-Path \$taskRoot 'hotd_model2\.exe'\) @taskArgs)",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(match, 'Could not find the production launch argument block')
        script = (
            "$taskRoms='C:\\Roms';$taskData='C:\\Data';"
            + match.group(1)
            + ';ConvertTo-Json -InputObject $taskArgs -Compress'
        )
        result = subprocess.run(
            [powershell, '-NoProfile', '-NonInteractive', '-Command', script],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        args = json.loads(result.stdout)
        self.assertEqual(len(args), 19, args)
        for name, folder in (
            ('nvram_directory', 'nvram'),
            ('cfg_directory', 'cfg'),
            ('diff_directory', 'diff'),
            ('state_directory', 'state'),
            ('snapshot_directory', 'snap'),
        ):
            with self.subTest(name=name):
                index = args.index('-' + name)
                self.assertEqual(args[index + 1], 'C:\\Data\\' + folder)


if __name__ == '__main__':
    unittest.main()
