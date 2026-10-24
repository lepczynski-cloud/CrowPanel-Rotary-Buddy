'use strict';

// Only offer firmware when the current deployment includes validated
// ESP32-S3 images produced by its automated Cloudflare Pages build.
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
    const len = binary.headers.get('content-length');
    if (!binary.ok || (len !== null && Number(len) < 65536)) {
      throw new Error('Published firmware missing or too small');
    }
    status.textContent = 'Firmware v' + json.version + ' ready.';
    status.classList.add('ready');
    button.hidden = false;
  } catch (_) {
    status.textContent = 'Installer temporarily unavailable. Please check the latest deployment or reload this page.';
    status.classList.add('missing');
    button.hidden = true;
  }
}

for (const option of images) checkImage(option);

for (const button of document.querySelectorAll('[data-preview]')) {
  button.addEventListener('click', () => {
    const mode = button.getAttribute('data-preview');
    document.getElementById('previewMedia').hidden = mode !== 'media';
    document.getElementById('previewBuddy').hidden = mode !== 'buddy';
    document.getElementById('deviceProduct').alt = mode === 'buddy'
      ? 'Illustration of the round CrowPanel rotary display with a smiling Buddy, coffee mug and COFFEE FIRST message.'
      : 'Illustration of the round CrowPanel rotary display showing the media controls preview.';
    for (const other of document.querySelectorAll('[data-preview]')) {
      const active = other === button;
      other.classList.toggle('active', active);
      other.setAttribute('aria-pressed', active ? 'true' : 'false');
    }
  });
}
