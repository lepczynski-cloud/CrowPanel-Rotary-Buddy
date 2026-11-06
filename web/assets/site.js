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
  status.classList.remove('ready', 'missing');
  button.hidden = true;
  try {
    const url = new URL(option.manifest, window.location.href);
    const response = await fetch(url, { cache: 'no-store' });
    if (!response.ok) throw new Error('Manifest HTTP ' + response.status);
    const json = await response.json();
    if (!Array.isArray(json.builds) || json.builds.length !== 1 ||
        json.builds[0].chipFamily !== 'ESP32-S3') {
      throw new Error('Unsupported ESP32 variant in release');
    }
    const parts = json.builds[0].parts;
    if (!Array.isArray(parts) || parts.length !== 1 || parts[0].offset !== option.offset ||
        parts[0].path !== option.expected) throw new Error('Unexpected firmware file or offset');
    // Pages publishes atomically from a verified build. Some CDNs reject HEAD
    // for .bin assets, so do not disable an otherwise valid installer on HEAD failure.
    const metadata = await fetch(new URL('./firmware/build-info.json', window.location.href), { cache:'no-store' });
    if (!metadata.ok) throw new Error('Build metadata HTTP ' + metadata.status);
    const info = await metadata.json();
    if (info.version !== json.version || info.chip !== 'ESP32-S3') {
      throw new Error('Firmware version mismatch');
    }
    status.textContent = 'Firmware v' + json.version + ' ready.';
    status.classList.add('ready');
    button.hidden = false;
  } catch (error) {
    status.textContent = 'Installer unavailable: ' + (error instanceof Error ? error.message : 'unknown error') + '. Try refreshing.';
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
    document.getElementById('previewHint').textContent = mode === 'media'
      ? 'Rotate for volume, tap to skip tracks or mute, press the knob to pause.'
      : 'Buddy reacts to taps, petting and idle time.';
    for (const other of document.querySelectorAll('[data-preview]')) {
      const active = other === button;
      other.classList.toggle('active', active);
      other.setAttribute('aria-pressed', String(active));
    }
  });
}

// Images are generated at deploy time from the linked GitHub repositories.
// No broken-image icon is shown if a repository has no suitable image.
for (const picture of document.querySelectorAll('[data-project-image]')) {
  const show = () => { if (picture.naturalWidth > 0) picture.classList.add('is-loaded'); };
  picture.addEventListener('load', show);
  if (picture.complete) show();
}
