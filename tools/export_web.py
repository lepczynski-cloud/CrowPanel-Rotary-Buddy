#!/usr/bin/env python3
"""Package a PlatformIO build for the ESP Web Tools browser installer.

Cloudflare Pages compiles firmware and runs this exporter on every Git push.
There is no manually published binary or local export step.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
ENV = 'crowpanel-rotary'
RELEASE_VERSION = '1.0.0'
FLASH_SIZE = 16 * 1024 * 1024
APP_OFFSET = 0x10000


def build_version(root: Path = ROOT) -> str:
    sha = os.environ.get('CF_PAGES_COMMIT_SHA') or os.environ.get('GITHUB_SHA')
    if not sha:
        try:
            result = subprocess.run(['git', 'rev-parse', '--short=7', 'HEAD'], cwd=root,
                                    capture_output=True, text=True, check=False)
            if result.returncode == 0:
                sha = result.stdout.strip()
        except OSError:
            pass
    if sha and re.fullmatch(r'[a-fA-F0-9]{7,40}', sha):
        return RELEASE_VERSION + '+g' + sha[:7].lower()
    return RELEASE_VERSION


def generate_manifest(image: str, offset: int, version=None) -> dict:
    manifest = {
        'name': 'CrowPanel Rotary Buddy',
        'version': version if version is not None else build_version(),
        'new_install_prompt_erase': True,
        'builds': [{'chipFamily': 'ESP32-S3', 'parts': [{'path': image, 'offset': offset}]}],
    }
    return manifest


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b''):
            digest.update(chunk)
    return digest.hexdigest()


def find_package(name: str) -> Path:
    candidates = [
        Path.home() / '.platformio' / 'packages' / name,
        ROOT / '.platformio' / 'packages' / name,
    ]
    for path in candidates:
        if path.is_dir():
            return path
    raise FileNotFoundError(f'Missing {name}. Build with PlatformIO first.')


def find_bootloader(build: Path, framework: Path) -> tuple[Path, str]:
    local = build / 'bootloader.bin'
    if local.is_file():
        return local, '80m'
    folder = framework / 'tools' / 'sdk' / 'esp32s3' / 'bin'
    for rate in ('80m', '40m'):
        candidate = folder / f'bootloader_dio_{rate}.bin'
        if candidate.is_file():
            return candidate, rate
    # Do not silently flash a bootloader for a different ESP32 family/mode.
    raise FileNotFoundError('ESP32-S3 DIO bootloader not found in the installed framework.')


def valid_esp_image(path: Path, minimum: int) -> None:
    if not path.is_file() or path.stat().st_size < minimum:
        raise ValueError(f'Missing or unexpectedly small firmware component: {path}')
    with path.open('rb') as stream:
        if stream.read(1) != b'\xe9':
            raise ValueError(f'Not an ESP32 application/bootloader image: {path}')


def export(root: Path, python_exe: str, dry_run: bool = False) -> dict:
    build = root / '.pio' / 'build' / ENV
    firmware = build / 'firmware.bin'
    partitions = build / 'partitions.bin'
    valid_esp_image(firmware, 65536)
    if not partitions.is_file() or not 512 <= partitions.stat().st_size <= 4096:
        raise ValueError(f'Invalid partition table: {partitions}')

    # Optional root override supports exporter fixture tests in CI/offline mode.
    framework = find_package('framework-arduinoespressif32') if root == ROOT else (
        root / '.platformio' / 'packages' / 'framework-arduinoespressif32')
    bootloader, rate = find_bootloader(build, framework)
    valid_esp_image(bootloader, 2048)
    boot_app0 = framework / 'tools' / 'partitions' / 'boot_app0.bin'
    if not boot_app0.is_file() or boot_app0.stat().st_size < 128:
        raise FileNotFoundError(f'Missing framework boot_app0.bin: {boot_app0}')

    website = root / 'web' / 'firmware'
    full_image = website / 'rotary-buddy.factory.bin'
    update_image = website / 'rotary-buddy.update.bin'
    command = [python_exe, '-m', 'esptool', '--chip', 'esp32s3', 'merge_bin',
               '--flash_mode', 'dio', '--flash_freq', rate, '--flash_size', '16MB',
               '-o', str(full_image),
               '0x0', str(bootloader),
               '0x8000', str(partitions),
               '0xe000', str(boot_app0),
               '0x10000', str(firmware)]
    if dry_run:
        return {'command': command, 'version': build_version(root), 'firmware': str(firmware)}

    website.mkdir(parents=True, exist_ok=True)
    print('Merging factory firmware using esptool:', flush=True)
    subprocess.run(command, check=True, cwd=root)
    shutil.copy2(firmware, update_image)
    valid_esp_image(full_image, 65536)
    valid_esp_image(update_image, 65536)
    if full_image.stat().st_size >= FLASH_SIZE or update_image.stat().st_size >= 0x400000:
        raise ValueError('Image exceeds the configured flash / app partition limit')

    version = build_version(root)
    factory = generate_manifest(full_image.name, 0, version)
    update = generate_manifest(update_image.name, APP_OFFSET, version)
    (website / 'manifest.json').write_text(json.dumps(factory, indent=2) + '\n', encoding='utf-8')
    (website / 'manifest-update.json').write_text(json.dumps(update, indent=2) + '\n', encoding='utf-8')
    entries = [
        f'{sha256_file(full_image)}  {full_image.name}',
        f'{sha256_file(update_image)}  {update_image.name}',
    ]
    (website / 'SHA256SUMS.txt').write_text('\n'.join(entries) + '\n', encoding='ascii')
    (website / 'build-info.json').write_text(json.dumps({
        'name': 'CrowPanel Rotary Buddy',
        'version': version,
        'chip': 'ESP32-S3',
        'factory_sha256': sha256_file(full_image),
        'update_sha256': sha256_file(update_image),
    }, indent=2) + '\n', encoding='utf-8')
    print(f'Published images to {website}:', flush=True)
    for entry in entries:
        print('  ' + entry)
    print('Ready for Cloudflare Pages to deploy the web/ directory.', flush=True)
    return {'factory': factory, 'update': update}


def main() -> int:
    parser = argparse.ArgumentParser(description='Prepare ESP Web Tools flashing images')
    parser.add_argument('--dry-run', action='store_true', help='Validate inputs without merging or writing output')
    args = parser.parse_args()
    try:
        export(ROOT, sys.executable, dry_run=args.dry_run)
        return 0
    except (ValueError, FileNotFoundError, subprocess.CalledProcessError, OSError) as error:
        print(f'ERROR: {error}', file=sys.stderr)
        print('Build the firmware with PlatformIO before exporting.', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
