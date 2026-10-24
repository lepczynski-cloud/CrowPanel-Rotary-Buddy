#!/usr/bin/env python3
"""Fail the build if the browser installer references invalid/stale binaries."""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import sys
from export_web import RELEASE_VERSION

EXPECTED = (
    ('manifest.json', 'rotary-buddy.factory.bin', 0),
    ('manifest-update.json', 'rotary-buddy.update.bin', 0x10000),
)


def verify(folder: Path) -> str:
    if not folder.is_dir():
        raise ValueError(f'Firmware directory is missing: {folder}')
    versions = set()
    sums = []
    for manifest_name, binary_name, offset in EXPECTED:
        manifest = json.loads((folder / manifest_name).read_text(encoding='utf-8'))
        if manifest.get('name') != 'CrowPanel Rotary Buddy':
            raise ValueError('Unexpected product name in manifest')
        build = manifest.get('builds')
        if not isinstance(build, list) or len(build) != 1 or build[0].get('chipFamily') != 'ESP32-S3':
            raise ValueError('Unexpected chip family or number of builds')
        if build[0].get('parts') != [{'path': binary_name, 'offset': offset}]:
            raise ValueError('Installer binary path or offset is incorrect')
        version = manifest.get('version')
        if not isinstance(version, str) or not version.startswith(RELEASE_VERSION):
            raise ValueError('Invalid release version')
        versions.add(version)
        binary = folder / binary_name
        if not binary.is_file() or binary.stat().st_size < 65536:
            raise ValueError(f'Binary missing or too small: {binary_name}')
        with binary.open('rb') as f:
            if f.read(1) != b'\xe9':
                raise ValueError(f'Invalid ESP image header: {binary_name}')
        digest = hashlib.sha256(binary.read_bytes()).hexdigest()
        sums.append(f'{digest}  {binary_name}')
    if len(versions) != 1:
        raise ValueError('The factory and update images have different versions')
    published = (folder / 'SHA256SUMS.txt').read_text(encoding='ascii').splitlines()
    if published != sums:
        raise ValueError('Checksums do not match firmware images')
    info = json.loads((folder / 'build-info.json').read_text(encoding='utf-8'))
    if info.get('version') != next(iter(versions)) or info.get('chip') != 'ESP32-S3':
        raise ValueError('Build info does not match the published manifests')
    if (info.get('factory_sha256'), info.get('update_sha256')) != (
        sums[0][:64], sums[1][:64]
    ):
        raise ValueError('Build metadata checksums are inconsistent')
    return next(iter(versions))


def main() -> int:
    target = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('web/firmware')
    try:
        print('Validated release:', verify(target))
        return 0
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print('RELEASE VERIFICATION FAILED:', exc, file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
