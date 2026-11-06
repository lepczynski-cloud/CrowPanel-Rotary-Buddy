#!/usr/bin/env python3
"""Fetch *real* project artwork from the authors' public GitHub repositories.

The output folder is gitignored. If images are unavailable, the HTML uses its
native text fallback rather than fake hardware photos or broken image icons.
"""
from __future__ import annotations
from dataclasses import dataclass
from io import BytesIO
from pathlib import Path
import json
import sys
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / 'web' / 'assets' / 'projects'
USER = 'lepczynski-cloud'
HEADERS = {'User-Agent': 'crowpanel-rotary-buddy-site-build', 'Accept': 'application/vnd.github+json'}

@dataclass(frozen=True)
class Source:
    key: str
    repo: str
    directory: str
    filename_hints: tuple[str, ...]

SOURCES = (
    Source('m5', 'M5Stack-Touch-Control-Deck', 'images/', ('device', 'finished', 'project', 'desk', 'case', 'control', 'm5stack', 'main')),
    Source('scratch', 'sb-to-html-converter', 'assets/readme/', ('screenshot', 'converter', 'preview', 'interface', 'app', 'main')),
    Source('ascii', 'ascii-camera-studio', 'public/assets/demo/', ('sample-ai-color-burst-ascii', 'ascii', 'demo')),
    Source('admin', 'admin-toolkit', 'public/assets/', ('screenshot', 'toolkit', 'dashboard', 'preview', 'site', 'og')),
)
EXTENSIONS = {'.png', '.jpg', '.jpeg', '.webp'}
MAX_IMAGE_BYTES = 12 * 1024 * 1024


def request_bytes(url: str, limit: int = MAX_IMAGE_BYTES) -> bytes:
    request = urllib.request.Request(url, headers=HEADERS)
    with urllib.request.urlopen(request, timeout=15) as response:
        content = response.read(limit + 1)
    if len(content) > limit:
        raise ValueError('Image download exceeds size cap')
    return content


def score_image(path: str, source: Source) -> int:
    name = Path(path).name.lower()
    low = path.lower()
    ext = Path(name).suffix
    if ext not in EXTENSIONS or not low.startswith(source.directory):
        return -1000
    if any(bad in name for bad in ('favicon','icon-','icon_','logo','qr','avatar','button','sprite','diagram','schematic','wiring','banner','badge')):
        return -1000
    score = 2
    score += 8 if ext in {'.jpg', '.jpeg', '.webp'} and source.key == 'm5' else 0
    score += sum(20 - idx for idx, hint in enumerate(source.filename_hints) if hint in name)
    score += 3 if 'readme' in low else 0
    # For Admin Toolkit prefer a complete page screenshot, not a tiny icon.
    if source.key == 'admin' and not any(h in low for h in ('screenshot','toolkit','preview','hero','og','dashboard')):
        return -1000
    return score


def choose_image(tree: list[dict], source: Source) -> str | None:
    items = [(score_image(entry.get('path', ''), source), entry.get('path', ''))
             for entry in tree if entry.get('type') == 'blob']
    candidates = [(n, p) for n, p in items if n > 0]
    if not candidates:
        return None
    candidates.sort(key=lambda item: (-item[0], item[1]))
    return candidates[0][1]


def convert_to_webp(raw: bytes, destination: Path) -> tuple[int, int]:
    from PIL import Image, ImageOps, UnidentifiedImageError
    with Image.open(BytesIO(raw)) as original:
        original.verify()
    with Image.open(BytesIO(raw)) as original:
        if original.format not in ('JPEG', 'PNG', 'WEBP'):
            raise ValueError('Unsupported image type')
        if original.width < 250 or original.height < 130 or original.width * original.height > 30000000:
            raise ValueError('Image is too small or too large')
        img = ImageOps.exif_transpose(original).convert('RGB')
        img.thumbnail((1200, 780), Image.Resampling.LANCZOS)
        destination.parent.mkdir(parents=True, exist_ok=True)
        img.save(destination, format='WEBP', quality=82, method=5)
        return img.size


def generate() -> dict[str, str]:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    # A fresh deployment must never accidentally publish old images.
    for old in OUTPUT.glob('*.webp'):
        old.unlink()
    result = {}
    for source in SOURCES:
        try:
            api = f'https://api.github.com/repos/{USER}/{source.repo}/git/trees/main?recursive=1'
            doc = json.loads(request_bytes(api, 3 * 1024 * 1024))
            if doc.get('truncated'):
                raise ValueError('GitHub tree was truncated')
            selected = choose_image(doc['tree'], source)
            if not selected:
                print(f'[Project image] No suitable screenshot in {source.repo}; using HTML fallback')
                continue
            encoded = '/'.join(urllib.parse.quote(part) for part in selected.split('/'))
            raw_url = f'https://raw.githubusercontent.com/{USER}/{source.repo}/main/{encoded}'
            size = convert_to_webp(request_bytes(raw_url), OUTPUT / f'{source.key}.webp')
            print(f'[Project image] {source.key}: {selected} -> {size[0]}x{size[1]} WebP')
            result[source.key] = selected
        except Exception as exc:
            print(f'[Project image] {source.key}: {exc}; safe HTML fallback')
    return result


if __name__ == '__main__':
    result = generate()
    print(f'[Project image] Prepared {len(result)}/{len(SOURCES)} real project previews')
