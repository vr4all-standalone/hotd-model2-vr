"""Original-ROM flash regression; owned fixtures remain outside distributables."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
PRIVATE = Path(os.environ.get('HOTD_MODEL2_PRIVATE_ROOT', 'D:/VR4ALL-HOTD-Model2-Build'))
CXX = os.environ.get('CXX', 'D:/msys64/mingw64/bin/g++.exe')


def function(source, signature):
    """Retain a complete production function body without rewriting its code."""
    start = source.index(signature)
    body = source.index('{', start)
    depth = 1
    end = body+1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]+'\n'


# Only native bitmap/device storage is substituted. Pixel selection, mask-pair
# scope, clipping and all draw loops below come from the real compiler source.
MASK_STORAGE = r'''
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cstdint>
#include <vector>
constexpr uint16_t TILEMAP_PIXEL_LAYER0=0x10;
constexpr int TILEMAP_DRAW_OPAQUE=0x10000;
using pen_t=uint32_t;
struct rectangle {int min_x,max_x,min_y,max_y;};
template<class T> struct bitmap_storage {
    int w,h;std::vector<T> storage;
    bitmap_storage(int width=512,int height=512):w(width),h(height),storage(size_t(w)*h){}
    T& pix(int y,int x=0){return storage.at(size_t(y)*w+x);}
    int rowpixels()const{return w;}
    int height()const{return h;}
};
using bitmap_ind16=bitmap_storage<uint16_t>;
using bitmap_ind8=bitmap_storage<uint8_t>;
using bitmap_rgb32=bitmap_storage<uint32_t>;
struct screen_device {bitmap_ind8 priority_map;bitmap_ind8& priority(){return priority_map;}};
struct fixture_palette {
    std::vector<pen_t> values=std::vector<pen_t>(65536);
    const pen_t* pens(){return values.data();}
};
struct fixture_tile {
    bitmap_ind16 pixels;bitmap_ind8 flags;
    bitmap_ind16& pixmap(){return pixels;}bitmap_ind8& flagsmap(){return flags;}
    void set_scrollx(int,int){}void set_scrolly(int,int){}
    void draw(screen_device&,bitmap_rgb32&,const rectangle&,int,int){throw std::runtime_error("Unexpected special scroll mode");}
};
struct segas24_tile_device {
    std::unique_ptr<uint16_t[]> tile_ram=std::make_unique<uint16_t[]>(0x8000);
    std::unique_ptr<fixture_tile> tile_layer[4];fixture_palette colors;
    segas24_tile_device(){for(auto& tile:tile_layer)tile=std::make_unique<fixture_tile>();}
    fixture_palette& palette(){return colors;}
    void draw_rect(screen_device&,bitmap_ind16&,bitmap_ind8&,bitmap_rgb32&,const uint16_t*,uint16_t,uint8_t,int,int,int,int,int,int,int,int);
    template<class BitmapClass> void draw_common(screen_device&,BitmapClass&,const rectangle&,int,int,int,bool=false,const uint16_t*=nullptr);
    void draw_with_window_mask(screen_device&,bitmap_rgb32&,const rectangle&,int,int,int,const uint16_t*,bool=false);
};
'''.replace('const uint16_t*=nullptr', 'const uint16_t* =nullptr')


class NativeFlashTests(unittest.TestCase):
    def build_run(self, native_mask=False):
        program = PRIVATE/'research/native-original-v1/program.bin'
        scenes = PRIVATE/'research/actual-scene-reference-v1/scenes'
        if not program.is_file() or not scenes.is_dir():
            self.skipTest('Private original-ROM and scene fixtures are unavailable')
        env = dict(os.environ)
        env['PATH'] = str(Path(CXX).parent)+os.pathsep+env.get('PATH', '')
        temporary_root = ROOT/'build/tests'
        temporary_root.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='hotd-flash-', dir=temporary_root) as temporary:
            binary = Path(temporary)/'native_flash_test.exe'
            arguments = []
            if native_mask:
                engine = PRIVATE/'engine/src/mame/sega/segaic24.cpp'
                if not engine.is_file():
                    self.skipTest('Private native tile renderer source is unavailable')
                source = engine.read_text(encoding='utf-8')
                harness = MASK_STORAGE+function(source, 'void segas24_tile_device::draw_rect(screen_device &screen, bitmap_ind16 &bm, bitmap_ind8 &tm, bitmap_rgb32 &dm')
                harness += 'template<class BitmapClass>\n'+function(source, 'void segas24_tile_device::draw_common(')
                harness += function(source, 'void segas24_tile_device::draw_with_window_mask(')
                (Path(temporary)/'native_mask_harness.h').write_text(harness, encoding='utf-8')
                arguments = ['-I'+temporary, '-DHOTD_FLASH_NATIVE_MASK_HARNESS="native_mask_harness.h"']
            build = subprocess.run([CXX, '-std=c++20', '-O2', '-UNDEBUG', *arguments,
                                    str(ROOT/'tests/native_flash_test.cpp'), '-o', str(binary)],
                                   env=env, capture_output=True, text=True, timeout=60)
            self.assertEqual(build.returncode, 0, build.stdout+build.stderr)
            run = subprocess.run([str(binary), str(program), str(scenes)],
                                 env=env, capture_output=True, text=True, timeout=30)
            self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
            print(run.stdout.strip())

    def test_original_capture_strobe_and_retained_fade(self):
        self.build_run()

    def test_actual_native_primary_mask_renderer_is_read_only(self):
        self.build_run(native_mask=True)


if __name__ == '__main__':
    unittest.main()
