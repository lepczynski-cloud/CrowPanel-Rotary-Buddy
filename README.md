# CrowPanel Rotary Buddy

**An animated desktop companion and USB media dial for the Elecrow 1.28-inch CrowPanel ESP32-S3 rotary touchscreen.**

[![MIT License](https://img.shields.io/badge/license-MIT-teal)](LICENSE)
![Hardware](https://img.shields.io/badge/hardware-ESP32--S3%20CrowPanel-blue)
![Firmware](https://img.shields.io/badge/firmware-v0.3.0--beta-orange)

Two swipeable screens. One universal volume knob. A tiny character with a lot of personality. No Arduino IDE, desktop agent, Wi-Fi account, cloud AI, or proprietary graphics files.

**Planned web installer:** <https://crowpanel-rotary-buddy.lepczynski.it/> (the domain is a deployment target, not necessarily live yet).

> **Status:** v0.3.0-beta is a new UI/firmware redesign built on the **previously hardware-tested v0.2.2 touch driver and HID configuration**. Offline C++ logic tests, full-source syntax validation against simulated APIs, and web installer checks have passed. A real device flash and a complete PlatformIO toolchain build are **not** confirmed in this release environment. Please build and test the firmware on your own CrowPanel before publishing a public installer binary.

## Features

### 1. MEDIA: a premium minimalist controller

- **Turn knob:** volume up/down on Windows, macOS, and Linux using standard USB Consumer HID reports.
- **Press knob:** Play/Pause in any mode.
- **Long-press knob:** switch between the two screens (backup navigation method).
- **Touch:** four large targets for Previous, Play/Pause, Next, and Mute.
- **Skip track:** animated glowing particles, sparks, and a traveling arc.
- **Play/Pause/Mute:** a short animated acknowledgement.
- **Adjust volume:** smooth, animated progress arc labeled **0–100% ESTIMATED**.

**A note on volume accuracy:** This device sends ordinary USB volume-up/volume-down key presses. HID has no reliable way to read the real Windows/macOS/Linux mixer level back from the host. The 0–100% ring is therefore an **estimate calculated locally** from turns, starting at 50%. Its reading can differ from the actual computer volume, especially after you use the computer's volume keys/mixer. A truly synchronized percentage requires optional host-side software, which this project intentionally does not depend on. The play/pause icon also represents a toggle, not verified player state.

### 2. BUDDY: an expressive desk companion

A procedural, original character with **43 animation and expression states**, including:

- Quiet idle, single/double blink, left/right/up/down glances, side-eye, eye rolls.
- Happy eyes, laughing, smiling, giggling, shyness, blush, surprise, confused, pouting, grumpy, mischief.
- Left/right wink, tongue, peekaboo, shy peek, yawn, sleepy face, sing, playful tears.
- Hearts, star eyes, glittering sparks, bubbles, rainbow arcs, dizzy spirals, light beams.
- Playful, screen-only remarks like **PSST!**, **HI...**, **COFFEE?** and **NICE!** (no audio or speech synthesis).
- Party, bouncing, thinking, idea, countdown, dreamy stars, sleep, wake-up.

**Personality pacing:** ~76% of random idle choices are subtle. Bigger moments are less frequent. It is *not* a loud or constantly moving slideshow. Tap the Buddy screen to trigger a different reaction. The engine runs entirely offline and does not imitate/redistribute any commercial robot's animation assets.

**Sleep lifecycle (default):**

| Situation | Behavior |
| --- | --- |
| 30 seconds of no input on MEDIA | Automatically open BUDDY. |
| Normal BUDDY idle | Relaxed glances and blinking, occasional playful reactions. |
| Five minutes of no input | Sleepy expression, then breathing/sleeping with rare dreams. |
| Tap, turn, press, or swipe | Wake up or respond; resume normal behavior. |
| Swipe left/right (also vertical) | Immediately switch between MEDIA and BUDDY. |

The timing and animation frequency are configurable in `include/Config.h` and `include/BuddyBrain.h`. The sleep state is an **animation**, not a USB sleep or power-off function. The screen stays on; brightness can be set in the firmware.

## Supported hardware

Only **Elecrow CrowPanel 1.28-inch HMI ESP32 Rotary Display, 240 × 240 IPS Round Touch Knob Screen**, with native USB **ESP32-S3**, GC9A01 SPI display, CST816D I2C touch and built-in quadrature encoder.

| Device interface | Hardware |
| --- | --- |
| Display | GC9A01, SPI: CLK 10, MOSI 11, DC 3, CS 9, RST 14 |
| Touch | CST816D: SDA 6, SCL 7, INT 5, RST 13 |
| Encoder | A 45, B 42, button 41 |
| Backlight | GPIO 46 |
| Power gate | GPIO 1 |
| Native USB | ESP32-S3 HID / ROM download port |
| Flash | 16 MB, factory application at `0x10000` |

Source: [official Elecrow Wiki](https://www.elecrow.com/wiki/CrowPanel_1.28inch-HMI_ESP32_Rotary_Display.html). **Not compatible** with the M5Stack CoreS3, generic ESP32-C3 rotary displays, or a random ESP32 board with different pin assignments.

## How to control it

| Input | MEDIA page | BUDDY page |
| --- | --- | --- |
| Turn encoder | Change computer volume, animate ring | Change computer volume, briefly show volume bubble |
| Short-press encoder | Play/Pause | Play/Pause |
| Long-press encoder | Switch to BUDDY | Switch to MEDIA |
| Swipe horizontally or vertically | Switch to BUDDY | Switch to MEDIA |
| Short tap on touchscreen | Activate one of four media buttons | Trigger a random playful expression |
| Leave untouched | Opens Buddy after 30 seconds | Slowly drifts into sleep after five minutes |

**Anti-accidental-action handling:** A swipe cannot also fire a media button. A valid media tap must start and end on the *same* button. When the large volume overlay is visible, the first screen tap dismisses it instead of activating an invisible media button. The encoder stays a volume controller on both pages.

The media HID usages work across modern Windows, macOS, and Linux desktop media-session implementations. Since the firmware contains **no OS-specific macros**, there is no host selection menu.

## Windows: build and flash WITHOUT Arduino IDE

**Requirements:** Windows 10/11; Python 3.9+ (with `venv` and pip); Internet connection **for first-time PlatformIO installation**; the original USB data cable; ESP32-S3 serial driver if needed. No Arduino IDE and no resident PC application.

1. Download or clone this project, then extract it into its own directory.
2. Run **`BUILD_WINDOWS.bat`**. On first use, the script installs PlatformIO inside a local `.venv` and downloads the pinned board packages and LovyanGFX.
3. Only if the build succeeds, run **`FLASH_WINDOWS.bat`**.
4. When prompted: **hold BOOT, tap RESET, release BOOT**. The ROM bootloader COM port should appear on Windows. Press Enter in the installer.
5. The script identifies the Espressif serial port, prompts if necessary, flashes the firmware, and reports success/failure.
6. Tap RESET once if the new firmware does not automatically start. The ESP32-S3 normally stops presenting a COM port after starting USB HID. This is expected.

Use `FLASH_WINDOWS.bat` again for later updates.

**CLion / PlatformIO CLI:** `pio run -e crowpanel-rotary`. Upload with `pio run -e crowpanel-rotary -t upload --upload-port COM7` (replace COM7). The `.bat` scripts and firmware remain independent of any IDE.

**macOS / Linux development:** install PlatformIO CLI, run `pio run -e crowpanel-rotary`, then upload with `pio run -t upload -e crowpanel-rotary --upload-port <serial-port>`. Native USB HID operation needs no desktop daemon on any supported OS.

## Browser firmware installer (ESP Web Tools)

This follows the **same overall mechanism** as [M5Stack Touch Control Deck](https://m5stack-touch-control-deck.lepczynski.it/): ESP Web Tools + Web Serial on HTTPS, one **application-only update** image and one **full factory** image. The static website is inside `web/`. An attractive preview is provided even before flashing binaries are built. The hero image in `web/assets/rotary-buddy-device.webp` is an **original illustrative render**, not an official Elecrow photograph or an exact screenshot of firmware output. The Buddy/Media switch on the webpage previews the two interfaces; it does not control a connected device. The realistic round metal dial and dark glass reflect the hardware form factor. No external image services are needed when the website is deployed.

> **IMPORTANT:** This repository does **not** contain a precompiled v0.3.0-beta binary in the initial source ZIP. The website correctly DISABLES both install buttons until it can find a compiled, published image. Do not upload an older v0.2.2 `.bin` under a v0.3.0-beta filename.

### Generate real web firmware

On Windows, **after confirming the PlatformIO build succeeds**, run:

```text
EXPORT_WEB_WINDOWS.bat
```

The script compiles the exact current source, installs pinned `esptool`, then creates:

```text
web/firmware/
  manifest.json                    # ESP32-S3 factory installer, offset 0
  manifest-update.json             # ESP32-S3 application-only updater, 0x10000
  rotary-buddy.factory.bin         # ESP32-S3 bootloader + partitions + boot_app0 + firmware
  rotary-buddy.update.bin          # app partition only; preserves NVS IF NOT erased
  SHA256SUMS.txt                    # verify the images you publish
```

The merged factory image uses the bootloader and the partition files from **your actual PlatformIO installation**, with the ESP32-S3 flash offsets taken from the Espressif tooling: bootloader `0x0`, partitions `0x8000`, boot app metadata `0xe000`, application `0x10000`. Firmware targets **16 MB / DIO**. Generated images should be manually flashed/tested on a real unit **before publishing**.

### Cloudflare Pages and the custom subdomain

1. Create a public GitHub repository named `CrowPanel-Rotary-Buddy` under `lepczynski-cloud`, and push this project's source.
2. After building and testing the firmware, **commit the five generated `web/firmware/` files** as well. They are necessary for the GitHub-integrated static installer to function. Avoid committing `.pio/`, `.venv/`, or intermediate build outputs.
3. In **Cloudflare > Workers & Pages > Create application > Pages**, connect the repository to a new project. Use the project root as root directory, **`exit 0`** as the no-op build command (if a command is required) and **`web`** as the output directory. No runtime backend or server is needed.
4. Add custom domain **`crowpanel-rotary-buddy.lepczynski.it`** to this *new* Pages project. Cloudflare provides HTTPS. This domain requires the owner's DNS/Pages configuration: adding the source files alone does not create the live domain.
5. Open the new site from Chrome/Edge on desktop. Use the included data USB cable and put ESP32-S3 into **BOOT + RESET** download mode.

The website checks that both manifest files and their `.bin` files exist before presenting the install buttons, so visitors do not see broken downloads.

### Update vs factory: choose carefully

- **Update:** flashes **only `firmware.bin` at `0x10000`**. This works for the same partition layout (the layout remains compatible with v0.2.2). **DO NOT choose `Erase device`** in the ESP Web Tools dialog if you want to preserve settings; the browser flasher may offer an erase even when you choose the update card.
- **Factory:** merged image at `0x0`, including bootloader, partition table and `boot_app0`. It resets stored configuration and is appropriate for a new device, a clean reset or recovery after incorrect partition changes.

ESP Web Tools uses browser Web Serial; it cannot flash from Safari/iOS and cannot run without HTTPS. Read the flashing dialog, verify the connected board, and do not disconnect the USB cable during flashing. See [ESP Web Tools documentation](https://esphome.github.io/esp-web-tools/) and [Espressif esptool reference](https://docs.espressif.com/projects/esptool/en/release-v4/esp32s3/esptool/basic-commands.html).

## Configuration

All tunable defaults are in `include/Config.h`:

| Parameter | Default | Meaning |
| --- | --- | --- |
| `AUTO_BUDDY_AFTER_MS` | 30000 | Idle MEDIA -> BUDDY delay |
| `SLEEP_AFTER_MS` | 300000 | Time without user input before sleepy/dream modes |
| `FRAME_INTERVAL_MS` | 45 | Approx. 22 graphical updates per second |
| `VOLUME_PULSES_PER_DETENT` | 3 | Normal volume pulses, before division |
| `FAST_VOLUME_PULSES` | 5 | Volume pulse acceleration while turning quickly |
| `VOLUME_SPEED_DIVISOR` | 2 | Matches the validated half-speed v0.2.2 feel |
| `VOLUME_INDICATOR_INITIAL_PERCENT` | 50 | Startup *estimated* volume |
| `VOLUME_INDICATOR_PERCENT_PER_REPORT` | 2 | Visual guess per USB volume pulse |
| `DEFAULT_BRIGHTNESS` | 85 | Screen backlight PWM percent |
| `USE_FRAMEBUFFER` | true | Smooth buffered draw, falls back if allocation fails |
| `TOUCH_SWIPE_MIN_PX` | 43 | Minimum deliberate swipe distance |

Behavior schedules and reaction choices are in `include/BuddyBrain.h`; all artwork and colors are in `include/BuddyGraphics.h`. You may customize them without graphic editors or binary animation assets. The character does **not** listen, speak, track music tempo, recognize you, read PC volume, or use AI; these would require additional hardware or software.

## Project files

```text
CrowPanel-Rotary-Buddy/
├── src/main.cpp                 USB HID, inputs, transitions and rendering loop
├── include/
│   ├── Config.h                 Pins and all editable settings
│   ├── CST816Touch.h            Known-good v0.2.2 polling touch driver
│   ├── RoundDisplay.h           Elecrow GC9A01 configuration
│   ├── TouchGesture.h           Safe tap/swipe distinction
│   ├── RotaryDecoder.h          Tested Gray-code decoder reference
│   ├── VolumeScaler.h           Tested signed volume speed scaling
│   ├── MediaLogic.h             Four hit zones and animated estimate
│   ├── BuddyBrain.h             43 moods, random schedule, sleep/wake
│   └── BuddyGraphics.h          Procedural GUI / face / particles
├── tests/                       Host logic and website release checks
├── tools/
│   ├── windows_cli.py           Windows build + flash + web-export helper
│   └── export_web.py            ESP Web Tools factory/update image packaging
├── web/                          Cloudflare Pages static website
│   ├── index.html
│   ├── assets/                   CSS/JS and original vector favicon
│   └── firmware/                 Populated when you export real binaries
├── platformio.ini
├── partitions.csv
├── BUILD_WINDOWS.bat
├── FLASH_WINDOWS.bat
├── EXPORT_WEB_WINDOWS.bat
├── LICENSE
└── README.md
```

There are **no GitHub Actions**, no mandatory backend, no external animation assets and no runtime dependency on Wi-Fi or cloud services.

## Testing and limitations

Host C++ tests (requires C++17 compiler such as GCC/Clang):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/host_tests.cpp -o /tmp/rotary-buddy-tests
/tmp/rotary-buddy-tests
python3 tests/test_web.py
node --check web/assets/site.js
```

These verify volume speed/direction, rotary Gray-code decoding, touch gestures, media zones, 0–100 clamping and animation smoothing, random moods, sleep/wake and website manifest offsets. Source-level C++ syntax was also checked using API stubs compatible with the prior firmware. **These checks are not equivalent to a real PlatformIO compilation or physical hardware test.** After flashing, verify touch, single swipe per page, encoder volume under heavy animation, wake/sleep, USB media keys and correct web installer offsets.

**Known limits:** The native HID stack cannot report actual system volume or track title. Rendered animations are procedurally drawn, not exported GIFs. USB must be connected for media control; Buddy animations still run if USB is providing power without a computer. Automatic host OS detection is not needed because only standard USB consumer media usages are emitted.

## GitHub metadata

**Repository name:** `CrowPanel-Rotary-Buddy`

**Description:** `An animated desktop companion and USB media dial for the CrowPanel ESP32-S3 rotary touchscreen. Two screens, expressive faces, browser flashing, no PC app.`

**Suggested topics:** `esp32-s3`, `elecrow`, `crowpanel`, `desktop-pet`, `desk-companion`, `rotary-encoder`, `usb-hid`, `media-controller`, `touchscreen`, `platformio`, `esp-web-tools`, `web-serial`, `windows`, `macos`, `linux`

## Credits and license

Made by [Wojciech Lepczyński](https://lepczynski.it/) as an independent open-source project. Inspired by the *idea* of expressive desk companions such as EMO and Eilik, but not affiliated with those products or their companies. Source code and original vector artwork are provided under the [MIT License](LICENSE). Uses the original public [Elecrow hardware documentation](https://www.elecrow.com/wiki/CrowPanel_1.28inch-HMI_ESP32_Rotary_Display.html), [LovyanGFX](https://github.com/lovyan03/LovyanGFX), and [ESP Web Tools](https://github.com/esphome/esp-web-tools), subject to their own licenses.
