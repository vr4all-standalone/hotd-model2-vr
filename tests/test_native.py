"""Compile and execute actual production C++ behavior; no mocks."""
import os
from pathlib import Path
import subprocess
import unittest

ROOT=Path(__file__).resolve().parents[1]
CXX=os.environ.get('CXX','D:/msys64/mingw64/bin/g++.exe')

class NativeTests(unittest.TestCase):
    def build_run(self,name):
        output=ROOT/'build'/'tests'/f'{name}.exe'
        output.parent.mkdir(parents=True,exist_ok=True)
        env=dict(os.environ);env['PATH']=str(Path(CXX).parent)+os.pathsep+env.get('PATH','')
        result=subprocess.run([CXX,'-std=c++20','-O2','-UNDEBUG',str(ROOT/'tests'/f'{name}.cpp'),'-o',str(output)],capture_output=True,text=True,env=env)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        result=subprocess.run([str(output)],capture_output=True,text=True,env=env)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        print(result.stdout.strip())
    def test_cabinet_protocol(self):self.build_run('cabinet_io_test')
    def test_unknown_program_is_closed(self):self.build_run('state_boundary_test')
    def test_input_settings(self):self.build_run('input_settings_test')
    def test_native_credit_acknowledgement(self):self.build_run('start_policy_test')
    def test_p2_remaining_ownership(self):self.build_run('p2_ownership_test')
    def test_only_committed_shots_drive_feedback(self):self.build_run('native_shots_test')
    def test_view_settings_and_shortcut_ownership(self):self.build_run('view_settings_test')
    def test_native_thread_uses_allowed_fast_cores(self):self.build_run('cpu_affinity_test')
    def test_grip_has_no_size_function(self):self.build_run('grip_shortcuts_removed_test')

if __name__=='__main__':unittest.main()
