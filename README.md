# CrowPanel Rotary Buddy

**An expressive desktop companion and premium USB media dial, built for the Elecrow CrowPanel 1.28-inch ESP32-S3 Rotary Display.**

[![License: MIT](https://img.shields.io/badge/license-MIT-teal.svg)](LICENSE)
[![ESP32-S3](https://img.shields.io/badge/board-ESP32--S3-1aadc8)](https://www.elecrow.com/wiki/CrowPanel_1.28inch-HMI_ESP32_Rotary_Display.html)
[![PlatformIO](https://img.shields.io/badge/firmware-PlatformIO-orange)](platformio.ini)
[![Browser installer](https://img.shields.io/badge/flash-from%20browser-7258ce)](https://crowpanel-rotary-buddy.lepczynski.it/)

<p align="center">
  <a href="https://crowpanel-rotary-buddy.lepczynski.it/"><img src="web/assets/readme-buddy.webp" alt="Buddy mode: the round rotary display with a smiling animated face, coffee icon and COFFEE FIRST speech bubble" width="48%"></a>
  <a href="https://crowpanel-rotary-buddy.lepczynski.it/"><img src="web/assets/readme-media.webp" alt="Media mode: the round rotary display showing previous, play/pause, next, mute and a glowing volume ring" width="48%"></a>
</p>
<p align="center"><sub>Buddy mode and Media mode. Illustrative previews of the website design, not photographs of the physical screen.</sub></p>
<p align="center">
  <a href="https://crowpanel-rotary-buddy.lepczynski.it/"><strong>Install from browser</strong></a> ·
  <a href="https://www.youtube.com/@WojciechLepczynski">YouTube</a> ·
  <a href="https://lepczynski.it/">Blog</a> ·
  <a href="https://github.com/lepczynski-cloud">More projects</a>
</p>

**Web installer:** https://crowpanel-rotary-buddy.lepczynski.it/  
**Source:** https://github.com/lepczynski-cloud/CrowPanel-Rotary-Buddy

Turn the knob to adjust volume, press it to play or pause, or use the round touchscreen to skip tracks and mute sound. When you're done, your dial becomes a little companion with animated eyes, silly expressions, tiny jokes and dreams. No Bluetooth pairing, desktop daemon, cloud access or Arduino IDE required.

The ESP32-S3 behaves as a standard **USB HID consumer-media controller**. It supports Windows, macOS and Linux (subject to the media-player behavior of the host OS), so there is no need to detect or configure an operating system profile.

## Two screens, one rotary dial

| MEDIA | BUDDY |
| --- | --- |
| Large **Previous**, **Play/Pause**, **Next** and **Mute** touch targets. Animated track-skip sparks, glowing rings and an animated 0–100% volume estimate. | **107 animated expression and effect states**, **108 rotating English captions**, responsive boops, blinking, laughter, rain, fireworks, coding jokes, dreams and a gentle sleep cycle. |

**Controls**

| Action | Result |
| --- | --- |
| Rotate the dial on either screen | Volume up/down (configurable sensitivity). |
| Short press the knob | Play/Pause. |
| Long press the knob, about 0.65 s | No action, to prevent accidental play/pause. |
| Swipe left or right | Switch between MEDIA and BUDDY. Only horizontal swipes navigate. |
| Stroke down on Buddy | Pet Buddy: hearts, big smiles, affectionate reactions and speech bubbles. No PC keys are sent. |
| Swipe up | Ignored to prevent accidental commands. |
| Tap a media button | Previous, Play/Pause, Next or Mute. The speaker icon displays sound waves or a muted slash. |
| Tap Buddy | Trigger a randomly selected expression or joke. |
| Leave MEDIA idle for 30 seconds | Automatically enter BUDDY mode. |
| Leave the device idle for 5 minutes | Buddy falls asleep and occasionally dreams or sleep-talks. |
| Tap, turn or press while Buddy sleeps | Wake it up. |

Buddy prefers gentle, infrequent movements, with louder reactions occurring less often. The face is slightly smaller so it can float and tilt. Speech appears in large, two-line comic-style bubbles; oversized coffee cups, Wi-Fi waves, search icons and hats can partially cover the face. Sample jokes include `LET'S GO PROD!`, `WORKS ON MINE!`, `CI SAYS YES!`, `BUG? FEATURE!`, `IS IT DNS?`, `COFFEE FIRST`, `FRIDAY DEPLOY?` and `ROLLBACK?`.

**These captions are visual jokes only.** They never open apps, run commands or deploy software to production.

### Meet Buddy's many moods

Buddy can be curious, sleepy, excited or in need of coffee. These **eight original illustrations from the project website** show the larger icons, outfits and speech bubbles. The expressions are rendered procedurally on the actual 240 × 240 display.

| Hello there! | Coffee first | DJ Buddy | Let's go prod! |
| :---: | :---: | :---: | :---: |
| <img src="web/assets/scenes/hello.svg" alt="Friendly Buddy face saying HELLO THERE!" width="185"> | <img src="web/assets/scenes/coffee.svg" alt="Buddy with a big coffee mug and COFFEE FIRST speech bubble" width="185"> | <img src="web/assets/scenes/headphones.svg" alt="Buddy wearing headphones and saying DJ BUDDY!" width="185"> | <img src="web/assets/scenes/crown.svg" alt="Buddy with a crown saying LET'S GO PROD!" width="185"> |
| **More pets!** | **Is it DNS?** | **Wi-Fi again?** | **Five more mins** |
| <img src="web/assets/scenes/pet.svg" alt="Happy Buddy reacting to a gentle pet with MORE PETS!" width="185"> | <img src="web/assets/scenes/dns.svg" alt="Buddy investigating with a magnifying glass, saying IS IT DNS?" width="185"> | <img src="web/assets/scenes/wifi.svg" alt="Buddy reacting to Wi-Fi with WI-FI AGAIN?" width="185"> | <img src="web/assets/scenes/sleep.svg" alt="Sleepy Buddy saying FIVE MORE MINS" width="185"> |

*Illustrations are representative artwork, not literal screenshots or promised word-for-word renderings of every firmware animation.* Explore the [interactive Buddy/Media preview](https://crowpanel-rotary-buddy.lepczynski.it/) for a closer look.

Firmware graphics are hand-coded C++ primitives rendered directly on the 240 × 240 round display. The website includes eight original SVG illustrations matching the Buddy style. No GIFs, cloud API, microphone, remote server or animation downloads are involved. The website's metallic product render is an original illustration, not a photograph of a finished unit.

**Important limitations:** standard USB HID can send volume commands but cannot read the actual system volume. The displayed 0–100% gauge is a local *estimate*, starting at a configurable value. Volume changes made directly on the computer will not be reflected automatically. The device does not receive song titles or playback status either. Similarly, its mute/unmute icon tracks only the last toggle sent **from the dial**, so muting directly on the computer may cause the icon to differ from the host state.

## Install firmware from your browser

1. Open **https://crowpanel-rotary-buddy.lepczynski.it/** in **Chrome or Edge on a desktop computer**.
2. Connect the **Elecrow CrowPanel 1.28-inch ESP32-S3 rotary dial** with its data-capable USB lead. Do not use the firmware with a different ESP32 device.
3. Enter flashing mode: **hold BOOT**, briefly press and release **RESET**, then release **BOOT**.
4. Choose **Factory installation** for a new or reset device, or **Update firmware** for an existing device with the same partition layout. Click the install button and select the ESP32-S3 serial port.
5. Follow the browser flashing dialog. Press **RESET** when flashing completes if the app does not start automatically.

The temporary serial port can disappear when the ESP32 boots as a USB HID device. This is normal. Everyday controls work without any installed PC software. The browser installer uses [ESP Web Tools](https://esphome.github.io/esp-web-tools/) and needs HTTPS and Web Serial support.

**Choose carefully:**

- **Update** flashes only the application at `0x10000`. It can retain settings, *provided you do not select "Erase device"* and the partition layout is unchanged.
- **Factory** writes a merged bootloader, partition table, boot metadata and application image beginning at `0x0`. To clear existing settings, select **Erase device** in the installer. This mode is intended for first installations and recovery.

## Automated GitHub → Cloudflare Pages builds

**Nothing needs to be compiled or exported manually after updating GitHub.** Every push to the GitHub `main` branch triggers Cloudflare Pages to compile the firmware and publish fresh web-flashing images alongside the website. This uses native **Cloudflare Pages Git integration**, not GitHub Actions, API tokens or a hosted backend.

```text
Edit code → git push main → Cloudflare Pages build
                                  |
                                  +-- C++ logic & rendering tests
                                  +-- Installer / release tests
                                  +-- PlatformIO firmware compilation
                                  +-- esptool factory + update image export
                                  +-- checksum and manifest verification
                                  +-- atomic web/ site publication
                                           |
                                           +-- browser can flash latest build
```

### One-time Cloudflare Pages setup

Create or open a **Cloudflare Pages project connected to this GitHub repository** (a Git-integrated Pages project, not a Workers static-assets build). Configure:

| Cloudflare Pages setting | Value |
| --- | --- |
| Production branch | `main` |
| Framework preset | `None` |
| **Build command** | **`bash scripts/build_pages.sh`** |
| Build output directory | `web` |
| Root directory | Repository root (leave the advanced field empty) |
| Build image | `v3` |
| Environment variables / API secrets | None required |
| Custom domain | `crowpanel-rotary-buddy.lepczynski.it` |

If your existing Pages project currently uses `exit 0` as its build command, **replace that with `bash scripts/build_pages.sh`** in **Workers & Pages → your project → Settings → Builds**. Keep the output folder `web`. Leave automatic production deployments enabled and confirm the repository is connected to the correct `main` branch.

After this one-time setup, simply push changes. Cloudflare downloads pinned build tools, compiles the application, creates the `.bin` files and manifests, and deploys everything together. The version shown on the installer includes the current Git commit's short hash, for example `1.1.0+gabcdef0` (the actual hash will vary).

Generated `web/firmware/` binaries are **intentionally gitignored**. They are produced during deployment and never need to be committed or copied by hand. When compilation, verification or any test fails, the Pages build exits nonzero and **the previous successful deployment remains available**. View failures under **Cloudflare Pages → Deployments → Build log**.

The first build needs Internet access and sufficient build time to download PlatformIO, the Espressif toolchain and LovyanGFX. Cloudflare Pages builds have a 20-minute timeout, so if downloads exceed that limit, inspect the build log. Hardware flashing and your Cloudflare account are outside this source repository; check the deployment logs for any compiler errors.

### What the build generates automatically

```text
web/firmware/
├── manifest.json                 ESP Web Tools factory manifest (offset 0)
├── manifest-update.json          ESP Web Tools update manifest (offset 0x10000)
├── rotary-buddy.factory.bin      Merged ESP32-S3 bootloader, partitions and application
├── rotary-buddy.update.bin       Application partition only
├── SHA256SUMS.txt                 SHA-256 checksums for both binaries
└── build-info.json               Git-derived build version and matching hashes
```

The two installers link to images generated **from the exact same Git commit as the published website**. The site checks the manifests and binaries before revealing the install buttons. The layout targets **ESP32-S3 with 16 MB flash in DIO mode**.

### Local developer build (optional)

You do not need any of these steps for GitHub-to-Cloudflare publication. To customize or debug the project, install Python 3.9 or newer on Windows and use:

- `BUILD_WINDOWS.bat` to create a local PlatformIO build.
- `FLASH_WINDOWS.bat` for direct USB flashing and automatic ESP32-S3 COM-port selection after BOOT + RESET.

On Linux/macOS with PlatformIO installed: `pio run -e crowpanel-rotary`. PlatformIO board settings and dependencies are in `platformio.ini`.

## Configuration

Edit `include/Config.h`. The defaults include:

| Key | Default | Description |
| --- | --- | --- |
| `AUTO_BUDDY_AFTER_MS` | `30000` | Media-to-Buddy idle transition. |
| `SLEEP_AFTER_MS` | `300000` | Sleep after five minutes with no user input. |
| `FRAME_INTERVAL_MS` | `45` | Approximately 22 animation frames per second. |
| `VOLUME_SPEED_DIVISOR` | `2` | Half-rate knob sensitivity. |
| `VOLUME_PULSES_PER_DETENT` | `3` | Normal USB volume pulse count before division. |
| `FAST_VOLUME_PULSES` | `5` | Fast-spin pulses before division. |
| `VOLUME_INDICATOR_INITIAL_PERCENT` | `50` | Starting local volume estimate. |
| `DEFAULT_BRIGHTNESS` | `85` | Backlight brightness as a percentage. |
| `TOUCH_SWIPE_MIN_PX` | `43` | Swipe distance threshold. |
| `USE_FRAMEBUFFER` | `true` | Smooth sprite drawing, with direct-render fallback. |

`include/BuddyBrain.h` contains the animation scheduler and 108 short English jokes. `include/BuddyGraphics.h` defines the animated speech bubbles, moving face, outfits, large icons, and display effects. Both files are ordinary C++, so adding your own mood requires no third-party graphics tool.

## Repository structure

```text
CrowPanel-Rotary-Buddy/
├── src/main.cpp                  USB HID, encoder, touch and animation loop
├── include/                     Drivers, gestures, configuration, graphics, moods
├── scripts/build_pages.sh       Automatic Cloudflare Pages compiler and release builder
├── tools/
│   ├── requirements-build.txt   Reproducible PlatformIO and esptool versions
│   ├── export_web.py            ESP32-S3 factory/update image generator
│   ├── verify_release.py        Checksums, manifests, binary safety checks
│   └── windows_cli.py           Optional Windows developer build/flash helper
├── tests/                       Host C++ tests and installer/release tests
├── web/
│   ├── index.html               Browser installer and project website
│   └── assets/                  Static design, illustrations and README previews
├── platformio.ini
├── partitions.csv
├── BUILD_WINDOWS.bat
├── FLASH_WINDOWS.bat
├── .gitignore
├── LICENSE
└── README.md
```

## Tests

The automated Pages build stops if a test or compiler step fails. Developers can also run the dependency-free tests locally:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/host_tests.cpp -o /tmp/rotary-host-tests
/tmp/rotary-host-tests

g++ -std=c++17 -O2 -Wall -Wextra -Werror -Itests/stubs -Iinclude tests/graphics_tests.cpp -o /tmp/rotary-graphics-tests
/tmp/rotary-graphics-tests

python3 -m unittest discover -s tests -p 'test_*.py' -v
node --check web/assets/site.js
```

Host tests exercise media touch areas, gestures, volume scaling, sleep/wake, all **107 renderable moods**, caption limits, random reactions, build-version consistency, safe flash offsets and checksum verification. They are not a substitute for full firmware compilation and a physical test on the target CrowPanel device. The actual hardware and Cloudflare Pages deployment must still be verified after the first online build.

## GitHub repository metadata

**Repository:** `CrowPanel-Rotary-Buddy`  
**Suggested description:** `An animated ESP32-S3 desktop buddy and USB media dial with 107 moods, 108 witty reactions and one-click browser flashing. Auto-built on Cloudflare Pages.`  
**Website:** `https://crowpanel-rotary-buddy.lepczynski.it/`

**Topics:** `esp32-s3`, `crowpanel`, `elecrow`, `desktop-pet`, `desk-companion`, `usb-hid`, `media-controller`, `rotary-encoder`, `touchscreen`, `platformio`, `esp-web-tools`, `web-serial`, `cloudflare-pages`, `open-source`, `windows`, `macos`, `linux`

## More projects from my workbench

I share practical hardware builds, local-first tools and automation experiments on my [YouTube channel](https://www.youtube.com/@WojciechLepczynski) and [DevOps & Cloud blog](https://lepczynski.it/). If you enjoyed Rotary Buddy, you might also like:

| Project | What it does | Try it | Source code |
| --- | --- | --- | --- |
| **M5Stack Touch Control Deck** | A compact touchscreen controller for PC actions and shortcuts. | [Web installer](https://m5stack-touch-control-deck.lepczynski.it/) | [GitHub](https://github.com/lepczynski-cloud/M5Stack-Touch-Control-Deck) |
| **SB to HTML Converter** | Run Scratch projects as standalone HTML, right in the browser. | [Open app](https://sbtohtml.lepczynski.it/) | [GitHub](https://github.com/lepczynski-cloud/sb-to-html-converter) |
| **ASCII Camera Studio** | Transform a camera feed, images or video into ASCII art. | [Open app](https://asciicamera.lepczynski.it/) | [GitHub](https://github.com/lepczynski-cloud/ascii-camera-studio) |
| **Online Admin Toolkit** | Useful browser tools for networking, Linux and everyday admin tasks. | [Open tools](https://admintoolkit.lepczynski.it/) | [GitHub](https://github.com/lepczynski-cloud/admin-toolkit) |

For more tutorials, projects and behind-the-scenes tests, visit **[lepczynski.it](https://lepczynski.it/)** or **[YouTube @WojciechLepczynski](https://www.youtube.com/@WojciechLepczynski)**.

## License and credits

MIT licensed. An independent open-source electronics project by [Wojciech Lepczyński](https://lepczynski.it/).

[Elecrow hardware information](https://www.elecrow.com/wiki/CrowPanel_1.28inch-HMI_ESP32_Rotary_Display.html) · [LovyanGFX](https://github.com/lovyan03/LovyanGFX) · [ESP Web Tools](https://github.com/esphome/esp-web-tools) · [Cloudflare Pages](https://developers.cloudflare.com/pages/)

The face, animation engine, jokes and artwork are original project content. External dependencies and hardware remain subject to their respective licenses and trademarks.
