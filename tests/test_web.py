"""Offline web-release structural tests; no ESP32 or Internet required."""
from __future__ import annotations

from html.parser import HTMLParser
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
from export_web import generate_manifest, APP_OFFSET, VERSION, valid_esp_image, export


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
        self.assertEqual(factory['version'], VERSION)
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

    def test_round_device_illustration(self):
        # The website now uses a local, round product illustration rather than
        # unrelated stock imagery. No network resource is needed to render it.
        asset = ROOT / 'web/assets/rotary-buddy-device.webp'
        data = asset.read_bytes()
        self.assertGreater(len(data), 20000)
        self.assertEqual(data[:4], b'RIFF')
        self.assertEqual(data[8:12], b'WEBP')
        html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
        script = (ROOT / 'web/assets/site.js').read_text(encoding='utf-8')
        self.assertIn('id="deviceProduct"', html)
        self.assertIn('id="previewMedia"', html)
        self.assertIn('rotary-buddy-device.webp', html)
        self.assertIn("document.getElementById('previewMedia').hidden", script)
        self.assertNotIn('previewBuddy', script)

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

    def test_partition_layout_consistency(self):
        partition = (ROOT / 'partitions.csv').read_text()
        self.assertIn('0x9000', partition)
        self.assertIn('0x10000', partition)
        self.assertIn('0x400000', partition)


if __name__ == '__main__':
    unittest.main(verbosity=2)
