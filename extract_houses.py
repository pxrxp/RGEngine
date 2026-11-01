"""Extract the ten largest named house groups from Houses.obj without dropping n-gons.

Usage:
  python3 extract_houses_fixed.py /path/to/Houses.obj [output_directory]

Coordinates match the original project's previous extractor: x*0.01, z*0.01, -y*0.01.
The output files are house_0.obj ... house_9.obj and keep the source group name in
line one so RivalGrounds.h can match each extracted visual mesh to Houses.obj.
"""
import sys
from collections import defaultdict
from pathlib import Path


def resolve_obj_index(raw, count):
    """OBJ indices are 1-based; negative indices are relative to current array end."""
    idx = int(raw)
    idx = idx - 1 if idx > 0 else count + idx
    if idx < 0 or idx >= count:
        raise ValueError(f"OBJ index {raw} outside array of length {count}")
    return idx


def parse_corner(token, nv, nvt, nvn):
    parts = token.split('/')
    vi = resolve_obj_index(parts[0], nv)
    vti = resolve_obj_index(parts[1], nvt) if len(parts) > 1 and parts[1] else None
    vni = resolve_obj_index(parts[2], nvn) if len(parts) > 2 and parts[2] else None
    return vi, vti, vni


def main():
    if len(sys.argv) < 2:
        raise SystemExit("usage: python3 extract_houses_fixed.py /path/to/Houses.obj [output_directory]")
    source_path = Path(sys.argv[1])
    output_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("resources")
    output_dir.mkdir(parents=True, exist_ok=True)

    verts, normals, texcoords = [], [], []
    group_faces = defaultdict(list)
    current_group = "default"

    with source_path.open("r", errors="ignore") as f:
        for line_no, raw_line in enumerate(f, 1):
            p = raw_line.strip().split()
            if not p or p[0].startswith('#'):
                continue
            tag = p[0]
            if tag == "v" and len(p) >= 4:
                x, y, z = map(float, p[1:4])
                verts.append((x * 0.01, z * 0.01, -y * 0.01))
            elif tag == "vn" and len(p) >= 4:
                nx, ny, nz = map(float, p[1:4])
                normals.append((nx, nz, -ny))
            elif tag == "vt" and len(p) >= 3:
                texcoords.append((float(p[1]), float(p[2])))
            elif tag in ("g", "o"):
                current_group = p[1] if len(p) > 1 else "default"
            elif tag == "f" and "house" in current_group.lower() and len(p) >= 4:
                try:
                    corners = [parse_corner(tok, len(verts), len(texcoords), len(normals))
                               for tok in p[1:]]
                except (ValueError, IndexError) as exc:
                    print(f"Skipping malformed face at {source_path}:{line_no}: {exc}", file=sys.stderr)
                    continue
                if len(corners) >= 3:
                    group_faces[current_group].append(corners)

    ranked = sorted(group_faces.items(), key=lambda item: len(item[1]), reverse=True)[:10]
    if not ranked:
        raise SystemExit("No groups with 'house' in their names and valid faces were found.")

    for out_idx, (group_name, faces) in enumerate(ranked):
        used_v, used_vt, used_vn = {}, {}, {}
        out_v, out_vt, out_vn = [], [], []
        out_faces = []

        for face in faces:
            out_face = []
            for vi, vti, vni in face:
                if vi not in used_v:
                    used_v[vi] = len(out_v) + 1
                    out_v.append(verts[vi])
                out_vti = None
                if vti is not None:
                    if vti not in used_vt:
                        used_vt[vti] = len(out_vt) + 1
                        out_vt.append(texcoords[vti])
                    out_vti = used_vt[vti]
                out_vni = None
                if vni is not None:
                    if vni not in used_vn:
                        used_vn[vni] = len(out_vn) + 1
                        out_vn.append(normals[vni])
                    out_vni = used_vn[vni]
