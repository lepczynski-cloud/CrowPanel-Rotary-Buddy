'use strict';

// Firmware verification follows the same two-manifest approach as the author's
// M5Stack Touch Control Deck web flasher. Never expose an install control if
// the corresponding real binary was not published to this deployment.
const images = [
  { id: 'update', manifest: './firmware/manifest-update.json', expected: 'rotary-buddy.update.bin', offset: 65536 },
  { id: 'factory', manifest: './firmware/manifest.json', expected: 'rotary-buddy.factory.bin', offset: 0 }
];

async function checkImage(option) {
  const status = document.getElementById(option.id + 'Status');
  const button = document.getElementById(option.id + 'Button');
  try {
    const url = new URL(option.manifest, window.location.href);
    const response = await fetch(url, { cache: 'no-store' });
    if (!response.ok) throw new Error('Manifest not deployed');
    const json = await response.json();
    if (!Array.isArray(json.builds) || json.builds.length !== 1 ||
        json.builds[0].chipFamily !== 'ESP32-S3') {
      throw new Error('Wrong chip family');
    }
    const parts = json.builds[0].parts;
    if (!Array.isArray(parts) || parts.length !== 1 || parts[0].offset !== option.offset ||
        parts[0].path !== option.expected) throw new Error('Unexpected binary');
    const binaryURL = new URL(parts[0].path, url);
    const binary = await fetch(binaryURL, { method: 'HEAD', cache: 'no-store' });
    if (!binary.ok || Number(binary.headers.get('content-length') || '0') < 65536) {
      throw new Error('Published firmware missing or too small');
    }
    status.textContent = 'Firmware v' + json.version + ' ready.';
    status.classList.add('ready');
    button.hidden = false;
  } catch (_) {
    status.textContent = 'Firmware binary not published yet. Build with EXPORT_WEB_WINDOWS.bat and deploy the web directory.';
    status.classList.add('missing');
    button.hidden = true;
  }
}

for (const option of images) checkImage(option);

for (const button of document.querySelectorAll('[data-preview]')) {
  button.addEventListener('click', () => {
    const mode = button.getAttribute('data-preview');
    document.getElementById('previewMedia').hidden = mode !== 'media';
    document.getElementById('deviceProduct').alt = mode === 'buddy'
      ? 'Illustration of the round CrowPanel rotary display: silver knurled dial, black circular touch screen and smiling cyan Buddy face.'
      : 'Illustration of the round CrowPanel rotary display showing the media controls preview.';
    for (const other of document.querySelectorAll('[data-preview]')) {
      const active = other === button;
      other.classList.toggle('active', active);
      other.setAttribute('aria-pressed', active ? 'true' : 'false');
    }
  });
}
