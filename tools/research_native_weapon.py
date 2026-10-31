"""Privately inspect HOTD's original model catalogue from an owned hotdo ZIP.

No game data is embedded in this tool. All generated assets belong in a private
research directory; the archive itself is read only.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import zipfile


def interleaved(archive, pairs, size):
    result = bytearray(size)
    for offset, first, second in pairs:
        a, b = archive.read(first), archive.read(second)
        assert len(a) == len(b) and len(a) % 2 == 0
        end = offset + len(a) * 2
        result[offset:end:4] = a[0::2]
        result[offset+1:end:4] = a[1::2]
        result[offset+2:end:4] = b[0::2]
        result[offset+3:end:4] = b[1::2]
    return result


def owned_regions(path):
    with zipfile.ZipFile(path) as archive:
        program = interleaved(archive, [(0, 'epr-19696.15', 'epr-19697.16'),
            (0x100000, 'epr-19694.13', 'epr-19695.14')], 0x200000)
        data = interleaved(archive, [(0, 'mpr-19704.11', 'mpr-19705.12'),
            (0x800000, 'mpr-19702.9', 'mpr-19703.10'),
            (0x1000000, 'mpr-19700.7', 'mpr-19701.8'),
            (0x1800000, 'epr-19698.5', 'epr-19699.6')], 0x2000000)
        for offset in range(0x1900000, 0x2000000, 0x100000):
            data[offset:offset+0x100000] = data[0x1800000:0x1900000]
        polygons = interleaved(archive, [(0, 'mpr-19715.17', 'mpr-19711.21'),
            (0x800000, 'mpr-19714.18', 'mpr-19710.22'),
            (0x1000000, 'mpr-19713.19', 'mpr-19709.23'),
            (0x1800000, 'mpr-19712.20', 'mpr-19708.24')], 0x2000000)
        textures = interleaved(archive, [(0, 'mpr-19718.27', 'mpr-19716.25'),
            (0x800000, 'mpr-19719.28', 'mpr-19717.26')], 0x1000000)
    if hashlib.sha256(program).hexdigest() != 'da2315b0b044d279728c8689336da6de0fee5997cf006514a1a633bd0de2fc75':
        raise ValueError('Not the supported original hotdo program')
    return program, data, polygons, textures


def catalogue(data):
    names = list(re.finditer(rb'PN_[a-zA-Z0-9_]+', data[0xE4B180:0xE73530]))
    return [dict(model=i, name=m.group().decode(),
        name_address=0x2E4B180+m.start(), entry=0x2E73530+i*16,
        tp=struct.unpack_from('<I', data, 0xE73530+i*16)[0],
        th=struct.unpack_from('<I', data, 0xE73534+i*16)[0],
        object=struct.unpack_from('<I', data, 0xE73538+i*16)[0],
        count=struct.unpack_from('<I', data, 0xE7353C+i*16)[0]) for i, m in enumerate(names)]


def decode_model(record, polygons, textures):
    at = (record['object'] & 0x7fffff) * 4
    if not record['object'] & 0x800000:
        raise ValueError('No ROM polygon object')
    p0 = struct.unpack_from('<3f', polygons, at)
    p1 = struct.unpack_from('<3f', polygons, at+12)
    at += 24
    tp, th = record['tp'] * 2, record['th'] * 2
    faces = []
    for link in range(record['count'] + 1):
        attr = struct.unpack_from('<I', polygons, at)[0]
        at += 4
        if not attr & 3:
            return faces
        normal = struct.unpack_from('<3f', polygons, at)
        p2 = struct.unpack_from('<3f', polygons, at+12)
        p3 = struct.unpack_from('<3f', polygons, at+24)
        at += 36
        count = 4 if attr & 1 else 3
        if count == 3:
            p3 = p2
        vertices = [p1, p0, p2, p3][:count]
        for vertex in vertices:
            if not all(math.isfinite(v) and abs(v) < 1e6 for v in vertex):
                raise ValueError('Invalid model vertex')
        header = struct.unpack_from('<4H', textures, th)
        uv = [struct.unpack_from('<2H', textures, tp+i*4)[::-1] for i in range(count)]
        if (attr >> 8) & 3:
            faces.append(dict(attr=attr, normal=normal, vertices=vertices, header=header, uv=uv))
        tp += count*4
        offset = (attr >> 12) & 31
        if offset & 16:
            offset -= 32
        th += offset*8
        kind = (attr >> 8) & 3
        if kind == 0 or kind == 2:
            p0, p1 = p2, p3
        elif kind == 1:
            p1 = p2
        else:
            p0 = p3
    raise ValueError('Missing native object terminator')


def projections(record, faces, output):
    from PIL import Image, ImageDraw
    size = 360
    image = Image.new('RGB', (size*3, size), (225, 228, 233))
    draw = ImageDraw.Draw(image)
    all_vertices = [v for f in faces for v in f['vertices']]
    for pane, (a, b, depth) in enumerate([(0, 1, 2), (0, 2, 1), (2, 1, 0)]):
        lo = [min(v[i] for v in all_vertices) for i in range(3)]
        hi = [max(v[i] for v in all_vertices) for i in range(3)]
        scale = (size-50)/max(hi[a]-lo[a], hi[b]-lo[b], 0.0001)
        for face in sorted(faces, key=lambda f: sum(v[depth] for v in f['vertices'])/len(f['vertices'])):
            points = [(pane*size+size/2+(v[a]-(lo[a]+hi[a])/2)*scale,
                size/2-(v[b]-(lo[b]+hi[b])/2)*scale) for v in face['vertices']]
            shade = int(110 + 65*abs(face['normal'][depth]))
            draw.polygon(points, fill=(shade, shade, shade), outline=(25, 30, 40))
        draw.text((pane*size+8, 7), f"{record['model']:04x} {record['name']} {'XYZ'[a]}{'XYZ'[b]}", fill=(0, 0, 0))
    image.save(output)


def authored_measurements(faces):
    # The first31 rendered faces form the original complete right-hand volume.
    # Round only for matching duplicated authored edge vertices, not centroid.
    hand = faces[:31]
    edges = {}
    volume, moment = 0.0, [0.0]*3
    for face in hand:
        vertices = face['vertices']
        for i, a in enumerate(vertices):
            b = vertices[(i+1) % len(vertices)]
            edge = tuple(sorted((tuple(round(x, 5) for x in a), tuple(round(x, 5) for x in b))))
            edges[edge] = edges.get(edge, 0)+1
        for i in range(1, len(vertices)-1):
            a, b, c = vertices[0], vertices[i], vertices[i+1]
            tetrahedron = sum(a[k]*(b[(k+1) % 3]*c[(k+2) % 3]-b[(k+2) % 3]*c[(k+1) % 3]) for k in range(3))/6
            volume += tetrahedron
            for k in range(3):
                moment[k] += tetrahedron*(a[k]+b[k]+c[k])/4
    if any(count != 2 for count in edges.values()) or abs(volume) < 1e-6:
        raise ValueError('Authored hand is not a closed nonzero volume')
    return dict(hand_faces=len(hand), hand_boundary_edges=0, hand_volume=volume,
        palm_volume_centroid=[v/volume for v in moment],
        muzzle_cap_face=45, muzzle_cap_centroid=[sum(v[k] for v in faces[45]['vertices'])/4 for k in range(3)],
        forward_source_axis='-Y', top_source_axis='+Z',
        presentation_metres_per_source_unit=0.1,
        scale_note='Physical presentation choice; source ROM geometry contains no metre units.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--models', nargs='+', type=lambda s: int(s, 0), default=[0x2C8, 0x3F4, 0x716, 0x719, 0x71B])
    parser.add_argument('--fixtures', action='store_true', help='Write private full ROM regions for the production C++ test')
    args = parser.parse_args()
    args.archive = args.archive.resolve()
    args.output = args.output.resolve()
    if args.output.is_relative_to(args.archive.parent):
        raise ValueError('Keep writable research separate from the original/canonical media directory')
    from game_files import validate_archives
    media = validate_archives([args.archive])
    if not media['ready']:
        raise ValueError('Owned HOTD media validation failed: '+json.dumps(media))
    program, data, polygons, textures = owned_regions(args.archive)
    records = catalogue(data)
    args.output.mkdir(parents=True, exist_ok=True)
    if args.fixtures:
        for name, region in [('program', program), ('main-data', data), ('polygons', polygons), ('textures', textures)]:
            (args.output/f'{name}.bin').write_bytes(region)
    (args.output/'catalogue.json').write_text(json.dumps(records, indent=2))
    summary = []
    for model in args.models:
        record = records[model]
        faces = decode_model(record, polygons, textures)
        projections(record, faces, args.output/f'model-{model:04x}.png')
        (args.output/f'model-{model:04x}.json').write_text(json.dumps(dict(record=record, faces=faces)))
        vertices = [v for f in faces for v in f['vertices']]
        summary.append(dict(record=record, rendered_faces=len(faces),
            bounds=[[min(v[i] for v in vertices) for i in range(3)], [max(v[i] for v in vertices) for i in range(3)]]))
    (args.output/'summary.json').write_text(json.dumps(summary, indent=2))
    evidence = dict(set='hotdo', media_verified=media['verified'], catalogue_models=len(records),
        region_sha256={name: hashlib.sha256(region).hexdigest() for name, region in
            [('program', program), ('main-data', data), ('polygons', polygons), ('textures', textures)]},
        measurements=authored_measurements(decode_model(records[0x719], polygons, textures)))
    (args.output/'evidence.json').write_text(json.dumps(evidence, indent=2))
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
