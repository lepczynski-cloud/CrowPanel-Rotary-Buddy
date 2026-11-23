"""Offline web-release structural tests; no ESP32 or Internet required."""
from __future__ import annotations

from html.parser import HTMLParser
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
from export_web import generate_manifest, APP_OFFSET, RELEASE_VERSION, build_version, valid_esp_image, export
from verify_release import verify


class HTMLAudit(HTMLParser):
    def __init__(self):
        super().__init__()
        self.ids: set[str] = set()
        self.buttons: list[dict] = []
        self.manifests: list[str] = []

    def handle_starttag(self, tag, attrs):
        props = dict(attrs)
        if 'id' in props:
            self.ids.add(props['id'])
        if tag == 'esp-web-install-button':
            self.manifests.append(props['manifest'])
        if tag == 'button':
            self.buttons.append(props)


class WebTests(unittest.TestCase):
    def test_manifest_offsets(self):
        factory = generate_manifest('rotary-buddy.factory.bin', 0)
        update = generate_manifest('rotary-buddy.update.bin', APP_OFFSET)
        self.assertEqual(factory['builds'][0]['chipFamily'], 'ESP32-S3')
        self.assertEqual(factory['builds'][0]['parts'][0]['offset'], 0)
        self.assertEqual(update['builds'][0]['parts'][0]['offset'], 0x10000)
        self.assertTrue(update['new_install_prompt_erase'])
        self.assertTrue(factory['version'].startswith(RELEASE_VERSION))
        self.assertTrue(json.loads(json.dumps(factory)))

    def test_static_web_installer(self):
        audit = HTMLAudit()
        audit.feed((ROOT / 'web/index.html').read_text(encoding='utf-8'))
        self.assertEqual(len(audit.manifests), 2)
        self.assertEqual(len(set(audit.manifests)), 2)
        self.assertIn('./firmware/manifest-update.json', audit.manifests)
        self.assertIn('./firmware/manifest.json', audit.manifests)
        self.assertTrue({'updateStatus', 'factoryStatus', 'updateButton', 'factoryButton'}.issubset(audit.ids))
        self.assertTrue((ROOT / 'web/assets/site.css').stat().st_size > 1000)
        self.assertTrue((ROOT / 'web/assets/site.js').stat().st_size > 200)

    def test_site_navigation_and_related_projects(self):
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        self.assertNotIn('More than eyes.', html)
        self.assertNotIn('id="reactions"', html)
        self.assertNotIn('href="#reactions"', html)
        self.assertIn('id="gallery"', html)
        self.assertIn('href="#gallery"', html)
        self.assertIn('id="projects"', html)
        self.assertIn('href="#projects"', html)
        self.assertIn('id="previewBuddy"', html)
        self.assertIn('id="previewMedia"', html)
        self.assertIn('src="./assets/scenes/coffee.svg" width="240" height="240"', html)
        self.assertIn('REAL HARDWARE, REAL SCREEN', html)
        for route in (
            'https://m5stack-touch-control-deck.lepczynski.it/',
            'https://sbtohtml.lepczynski.it/',
            'https://asciicamera.lepczynski.it/',
            'https://admintoolkit.lepczynski.it/',
            'https://github.com/lepczynski-cloud/M5Stack-Touch-Control-Deck',
            'https://github.com/lepczynski-cloud/sb-to-html-converter',
            'https://github.com/lepczynski-cloud/ascii-camera-studio',
            'https://github.com/lepczynski-cloud/admin-toolkit',
            'https://lepczynski.it/',
            'https://www.youtube.com/@WojciechLepczynski',
        ):
            self.assertIn(route, html)

    def test_real_product_photos_and_separate_ui_preview(self):
        # The two user-provided photos are self-hosted optimized WebPs, and
        # illustrative UI previews are intentionally labelled as such.
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        script = (ROOT / 'web/assets/site.js').read_text(encoding='utf-8')
        readme = (ROOT / 'README.md').read_text(encoding='utf-8')
        for name in ('rotary-buddy-real.webp', 'rotary-buddy-dns.webp'):
            asset = ROOT / 'web/assets' / name
            data = asset.read_bytes()
            self.assertGreater(len(data), 12000)
            self.assertEqual(data[:4], b'RIFF')
            self.assertEqual(data[8:12], b'WEBP')
            self.assertIn(name, html)
        self.assertIn('id="previewMedia"', html)
        self.assertIn('id="previewBuddy"', html)
        hero = html.split('<div class="hero-visual"', 1)[1].split('</section>', 1)[0]
        self.assertIn('class="hero-photo photo-media"', hero)
        self.assertIn('src="./assets/readme-media.webp"', hero)
        self.assertIn('MEDIA MODE', hero)
        self.assertIn('UI PREVIEW', hero)
        self.assertIn('rotary-buddy-real.webp', hero)
        self.assertIn('rotary-buddy-dns.webp', hero)
        self.assertIn('UI PREVIEW', html)
        self.assertIn("document.getElementById('previewMedia').hidden", script)
        self.assertIn("document.getElementById('previewBuddy').hidden", script)
        self.assertNotIn('rotary-buddy-device.webp', html)

    def test_gallery_faces_are_valid_local_svg(self):
        from xml.etree import ElementTree as ET
        folder = ROOT / 'web/assets/scenes'
        scenes = sorted(folder.glob('*.svg'))
        self.assertEqual(len(scenes), 10)
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        for svg in scenes:
            root = ET.fromstring(svg.read_text(encoding='utf-8'))
            self.assertTrue(root.tag.endswith('svg'))
            self.assertIn('viewBox', root.attrib)
            self.assertIn('assets/scenes/' + svg.name, html)
        self.assertIn('mood-gallery', html)
        self.assertIn('SWIPE', html.upper())

    def test_local_project_images_are_bundled(self):
        # Screenshots and the device photo must be shipped with the source.
        # Cloudflare builds must not depend on public GitHub image downloads.
        from PIL import Image
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        for key in ('m5', 'scratch', 'ascii', 'admin'):
            path = ROOT / 'web/assets/projects' / f'{key}.webp'
            self.assertTrue(path.is_file(), f'Missing real project image: {key}')
            self.assertGreater(path.stat().st_size, 8000)
            with Image.open(path) as image:
                self.assertEqual(image.format, 'WEBP')
                self.assertEqual(image.size, (960, 540))
                image.verify()
            self.assertIn(f'./assets/projects/{key}.webp', html)
        css = (ROOT / 'web/assets/site.css').read_text(encoding='utf-8')
        self.assertIn('object-position:center;opacity:1', css)
        self.assertNotIn('web/assets/projects/', (ROOT / '.gitignore').read_text())
        self.assertNotIn('fetch_project_images.py', (ROOT / 'scripts/build_pages.sh').read_text())
        self.assertEqual(html.count('data-project-image'), 4)

    def test_readme_photos_square_and_video_links(self):
        from PIL import Image
        readme = (ROOT / 'README.md').read_text(encoding='utf-8')
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        for name in ('readme-photo-buddy.webp', 'readme-photo-dns.webp'):
            path = ROOT / 'web/assets' / name
            self.assertIn(name, readme)
            with Image.open(path) as image:
                self.assertEqual(image.size, (680, 680))
                image.verify()
        self.assertIn('Watch the Rotary Buddy video', readme)
        self.assertIn('https://www.youtube.com/watch?v=vjOF7N7Cmbo', readme)
        self.assertIn('id="video"', html)
        self.assertIn('href="#video"', html)
        self.assertIn('https://www.youtube-nocookie.com/embed/vjOF7N7Cmbo', html)
        self.assertIn('loading="lazy"', html)
        self.assertIn('allowfullscreen', html)
        self.assertIn('video-frame', (ROOT / 'web/assets/site.css').read_text())

    def test_media_layout_labels_below_icons_and_clean_background(self):
        graphics = (ROOT / 'include/BuddyGraphics.h').read_text(encoding='utf-8')
        self.assertIn('text(g, "MEDIA", 120, 34, 4, WHITE)', graphics)
        self.assertIn('text(g, "R O T A R Y   B U D D Y", 120, 63, 1, MUTED)', graphics)
        self.assertIn('text(g, "PREV", 47, 165, 1, MUTED)', graphics)
        self.assertIn('text(g, "NEXT", 193, 165, 1, MUTED)', graphics)
        self.assertIn('"MUTE KEY SENT" : "MUTE"', graphics)
        self.assertIn('"USB HID CONTROL"', graphics)
        self.assertNotIn('"%d%%"', graphics)
        self.assertNotIn('"ESTIMATED"', graphics)
        self.assertNotIn('g.fillCircle(28, 178, 1, GOLD)', graphics)
        self.assertNotIn('g.fillCircle(205, 57, 1, VIOLET)', graphics)
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        self.assertIn('readme-media.webp', html)
        self.assertIn('data-preview="media"', html)
        self.assertIn('data-preview="buddy"', html)

    def test_exporter_fails_closed_on_missing_binary(self):
        with self.assertRaises(ValueError):
            valid_esp_image(ROOT / 'web/firmware/no-such-file.bin', 65536)

    def test_dry_run_export_targets_and_factory_update_paths(self):
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            build = root / '.pio' / 'build' / 'crowpanel-rotary'
            build.mkdir(parents=True)
            framework = root / '.platformio' / 'packages' / 'framework-arduinoespressif32'
            bootapp = framework / 'tools' / 'partitions' / 'boot_app0.bin'
            bootapp.parent.mkdir(parents=True)
            bootapp.write_bytes(b'boot_app0' * 128)
            firmware = build / 'firmware.bin'
            firmware.write_bytes(b'\xe9' + bytes(131072 - 1))
            partitions = build / 'partitions.bin'
            partitions.write_bytes(b'PARTITIONS' * 256)
            bootloader = build / 'bootloader.bin'
            bootloader.write_bytes(b'\xe9' + bytes(8192 - 1))
            plan = export(root, sys.executable, dry_run=True)
            cmd = plan['command']
            self.assertEqual(cmd[cmd.index('--chip') + 1], 'esp32s3')
            self.assertIn('--flash_size', cmd)
            self.assertEqual(cmd[cmd.index('--flash_size') + 1], '16MB')
            self.assertEqual(cmd[cmd.index('0x0') + 1], str(bootloader))
            self.assertEqual(cmd[cmd.index('0x8000') + 1], str(partitions))
            self.assertEqual(cmd[cmd.index('0xe000') + 1], str(bootapp))
            self.assertEqual(cmd[cmd.index('0x10000') + 1], str(firmware))
            self.assertFalse((root / 'web').exists())

            # Simulate successful merge_bin rather than claiming toolchain compiled.
            def stub_merge(command, check, cwd):
                self.assertTrue(check)
                self.assertEqual(cwd, root)
                Path(command[command.index('-o') + 1]).write_bytes(
                    b'\xe9' + bytes(1024 * 1024 - 1))

            with patch.dict(os.environ, {'CF_PAGES_COMMIT_SHA': 'abc1234de00000112233'}):
                with patch('export_web.subprocess.run', side_effect=stub_merge):
                    release = export(root, sys.executable)
            output = root / 'web' / 'firmware'
            self.assertTrue((output / 'rotary-buddy.factory.bin').is_file())
            self.assertEqual((output / 'rotary-buddy.update.bin').read_bytes(), firmware.read_bytes())
            self.assertEqual(release['factory']['builds'][0]['parts'][0]['offset'], 0)
            self.assertEqual(release['update']['builds'][0]['parts'][0]['offset'], 0x10000)
            self.assertTrue((output / 'SHA256SUMS.txt').is_file())
            self.assertEqual(len((output / 'SHA256SUMS.txt').read_text().splitlines()), 2)
            self.assertTrue(json.loads((output / 'manifest.json').read_text()))
            self.assertEqual(verify(output), RELEASE_VERSION + '+gabc1234')
            with (output / 'rotary-buddy.update.bin').open('ab') as stream:
                stream.write(b'wrong')
            with self.assertRaisesRegex(ValueError, 'Checksums'):
                verify(output)

    def test_git_version_and_automatic_build_contract(self):
        with patch.dict(os.environ, {'CF_PAGES_COMMIT_SHA': 'DEADBEEF001122'}):
            self.assertEqual(build_version(), RELEASE_VERSION + '+gdeadbee')
        runner = (ROOT / 'scripts/build_pages.sh').read_text(encoding='utf-8')
        self.assertIn('set -Eeuo pipefail', runner)
        self.assertIn('python -m platformio run -e crowpanel-rotary', runner)
        self.assertIn('python tools/export_web.py', runner)
        self.assertIn('python tools/verify_release.py web/firmware', runner)
        self.assertIn('rm -rf web/firmware', runner)
        ignore = (ROOT / '.gitignore').read_text(encoding='utf-8')
        self.assertIn('web/firmware/', ignore)
        self.assertIn('.build-venv/', ignore)

    def test_site_describes_automatic_installer(self):
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        js = (ROOT / 'web/assets/site.js').read_text(encoding='utf-8')
        self.assertIn('113 animated moods', html)
        self.assertIn('135 witty captions', html)
        self.assertIn('Cloudflare automatically builds', html)
        self.assertNotIn("method: 'HEAD'", js)
        self.assertFalse(list((ROOT / '.github/workflows').glob('*.yml')))
        self.assertNotIn('not published yet', html + js)
        self.assertNotIn('EXPORT_WEB_WINDOWS.bat', html + js)

    def test_driverless_cross_platform_media_protocol(self):
        code = (ROOT / 'src/main.cpp').read_text()
        readme = (ROOT / 'README.md').read_text()
        self.assertIn('#include <USBHIDConsumerControl.h>', code)
        self.assertNotIn('#include <USBHIDKeyboard.h>', code)
        for k in ('CONSUMER_CONTROL_VOLUME_INCREMENT',
                  'CONSUMER_CONTROL_VOLUME_DECREMENT',
                  'CONSUMER_CONTROL_PLAY_PAUSE',
                  'CONSUMER_CONTROL_SCAN_NEXT',
                  'CONSUMER_CONTROL_SCAN_PREVIOUS',
                  'CONSUMER_CONTROL_MUTE'):
            self.assertIn(k, code)
        for os_name in ('Windows', 'macOS', 'Linux'):
            self.assertIn(os_name, readme)
        self.assertIn('no volume percentage', readme.lower())
        self.assertIn('without a misleading percentage', (ROOT / 'web/index.html').read_text())
        self.assertNotIn('0 to 100% (estimated', (ROOT / 'web/index.html').read_text())

    def test_partition_layout_consistency(self):
        partition = (ROOT / 'partitions.csv').read_text()
        self.assertIn('0x9000', partition)
        self.assertIn('0x10000', partition)
        self.assertIn('0x400000', partition)


if __name__ == '__main__':
    unittest.main(verbosity=2)
