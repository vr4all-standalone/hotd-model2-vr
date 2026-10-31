"""Run the production host on one complete private owned-HOTD scene capture.

No game geometry, textures, rasters or ROM bytes belong in public source.
Selected geometry interchange follows the GPL VC2 diagnostic replay tool.
"""
import argparse
import json
from pathlib import Path
import subprocess


def geometry(data):
    rows = [[data[k] for k in ('sequence', 'width', 'height', 'xoffset', 'yoffset')]
            + [len(data['polygons'])]]
    for p in data['polygons']:
        row = [len(p['vertices'])] + [p[k] for k in ('attr', 'window', 'sort', 'luma', 'lod', 'object')]
        row += [int(p['culled'])] + p['header'] + p['focus'] + p['center'] + p['viewport']
        rows.append(row + [value for vertex in p['vertices'] for value in vertex])
    return '\n'.join(' '.join(str(v) for v in row) for row in rows) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--metadata', type=Path, required=True)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--host', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    metadata = args.metadata.resolve()
    data = json.loads(metadata.read_text(encoding='utf-8'))
    if metadata.name != 'metadata.json' or data.get('schema') != 1 or data['rejected']:
        raise ValueError('Requires a complete unrejected private native capture')
    if not data['polygons'] or len(data['polygons']) > 32768:
        raise ValueError('Requires bounded actual game geometry')
    args.output.mkdir(parents=True, exist_ok=True)
    # A fresh output per run prevents stale artifacts from satisfying the wait.
    import tempfile
    with tempfile.TemporaryDirectory(prefix='captured-host-', dir=args.output) as run:
        run = Path(run)
        text = run/'geometry.txt'
        text.write_text(geometry(data), encoding='utf-8')
        Path(str(text)+'.culls').write_text('\n'.join(str(p['cull_reasons']) for p in data['polygons']), encoding='utf-8')
        result = subprocess.run([str(args.exe.resolve()), str(text), str(metadata.parent),
                                 str(args.host.resolve()), str(run)], capture_output=True, text=True, timeout=30)
        print(result.stdout, end='')
        print(result.stderr, end='')
        # Keep only the actual logs/images as evidence outside the temp folder.
        import shutil
        for file in run.iterdir():
            if file.suffix in {'.ppm', '.log'}:
                shutil.copy2(file, args.output/file.name)
        result.check_returncode()


if __name__ == '__main__':
    main()
