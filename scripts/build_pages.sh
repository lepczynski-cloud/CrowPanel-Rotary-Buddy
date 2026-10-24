#!/usr/bin/env bash
# Cloudflare Pages Git integration: run this once for EVERY GitHub push.
# It builds the ESP32-S3 firmware, validates it, and publishes a website
# containing the browser-flashable factory and update images.
set -Eeuo pipefail
cd "$(dirname "$0")/.."

echo "[Rotary Buddy] Starting Cloudflare Pages build"
python3 --version

# Keep all downloaded compilers, Python packages, and firmware artifacts out
# of the Git repository and web output.
python3 -m venv .build-venv
. .build-venv/bin/activate
python -m pip install --disable-pip-version-check --no-input -r tools/requirements-build.txt

# Do not allow an outdated binary from a previous run into this deployment.
rm -rf web/firmware

echo '[Rotary Buddy] Running C++ behavioral tests'
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/host_tests.cpp -o .build-venv/rotary-buddy-tests
.build-venv/rotary-buddy-tests

g++ -std=c++17 -O2 -Wall -Wextra -Werror -Itests/stubs -Iinclude \
    tests/graphics_tests.cpp -o .build-venv/rotary-buddy-graphics-tests
.build-venv/rotary-buddy-graphics-tests

echo '[Rotary Buddy] Running browser installer and release tests'
python -m unittest discover -s tests -p 'test_*.py' -v
if command -v node >/dev/null 2>&1; then
    node --check web/assets/site.js
fi

echo '[Rotary Buddy] Compiling ESP32-S3 with PlatformIO'
python -m platformio run -e crowpanel-rotary

echo '[Rotary Buddy] Creating signed-by-checksum web-flash images'
python tools/export_web.py

echo '[Rotary Buddy] Verifying published installer payload'
python tools/verify_release.py web/firmware

echo '[Rotary Buddy] All stages passed, Cloudflare Pages may deploy web/'
