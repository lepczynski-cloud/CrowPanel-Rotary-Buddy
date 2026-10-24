"""Self-contained PlatformIO build, upload and website export helper for Windows.

Python 3.9+ is required. The first run creates .venv locally and installs a
pinned PlatformIO version. Upload mode compiles first, then asks the user to
enter the ESP32-S3 ROM download mode and automatically looks for its COM port.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
import os
from pathlib import Path
import subprocess
import sys
import time
from typing import Optional

ROOT = Path(__file__).resolve().parent.parent
VENV = ROOT / ".venv"
PYTHON = VENV / "Scripts" / "python.exe"
PIO = VENV / "Scripts" / "platformio.exe"
PIO_VERSION = "6.1.18"
ENVIRONMENT = "crowpanel-rotary"
ESPRESSIF_USB_VID = 0x303A


@dataclass(frozen=True)
class PortInfo:
    device: str
    description: str
    hwid: str
    vid: Optional[int]
    pid: Optional[int]


def run_command(args: list[str]) -> int:
    print("\n> " + " ".join(str(x) for x in args), flush=True)
    try:
        return subprocess.call(args, cwd=ROOT)
    except OSError as exc:
        print(f"ERROR: Could not start process: {exc}", flush=True)
        return 1


def prepare_environment() -> bool:
    if sys.version_info < (3, 9):
        print("ERROR: Python 3.9 or later is required.")
        return False

    if not PYTHON.exists():
        print("[Setup] Creating an isolated Python environment in .venv ...")
        if run_command([sys.executable, "-m", "venv", str(VENV)]) != 0:
            print("ERROR: Python venv creation failed. Reinstall Python with pip/venv.")
            return False

    check = subprocess.run(
        [str(PYTHON), "-c",
         f"import importlib.metadata as m; assert m.version('platformio') == '{PIO_VERSION}'"],
        cwd=ROOT,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    if check.returncode != 0 or not PIO.exists():
        print("[Setup] Installing PlatformIO. First run requires Internet access.")
        if run_command([str(PYTHON), "-m", "pip", "install", "--disable-pip-version-check",
                        f"platformio=={PIO_VERSION}"]) != 0:
            print("ERROR: PlatformIO installation failed. Check network/proxy settings.")
            return False
    return True


def available_ports() -> list[PortInfo]:
    """Read COM port metadata using pyserial from the project virtualenv."""
    program = (
        "import json; from serial.tools.list_ports import comports; "
        "print(json.dumps([{'device':p.device,'description':p.description or 'Serial device',"
        "'hwid':p.hwid or '', 'vid':p.vid, 'pid':p.pid} for p in comports()]))"
    )
    result = subprocess.run(
        [str(PYTHON), "-c", program],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        print("ERROR: Unable to inspect COM ports:", result.stderr.strip())
        return []
    try:
        records = json.loads(result.stdout or "[]")
        return [PortInfo(**record) for record in records]
    except (json.JSONDecodeError, TypeError) as exc:
        print(f"ERROR: Invalid COM port data: {exc}")
        return []


def is_espressif_port(port: PortInfo) -> bool:
    if port.vid == ESPRESSIF_USB_VID:
        return True
    identity = f"{port.description} {port.hwid}".upper()
    hints = ("ESP32", "ESPRESSIF", "USB JTAG", "USB SERIAL/JTAG")
    return any(hint in identity for hint in hints)


def print_ports(ports: list[PortInfo]) -> None:
    if not ports:
        print("No serial ports found.")
        return
    print("Detected serial ports:")
    for index, port in enumerate(ports, start=1):
        usb_id = ""
        if port.vid is not None and port.pid is not None:
            usb_id = f" [{port.vid:04X}:{port.pid:04X}]"
        marker = "  ESP32" if is_espressif_port(port) else ""
        print(f"  {index}. {port.device:8} {port.description}{usb_id}{marker}")


def auto_detect_download_port(before: set[str], attempts: int = 24) -> Optional[str]:
    """Prefer a newly appeared Espressif port, then any Espressif port."""
    for _ in range(attempts):
        ports = available_ports()
        new_ports = [p for p in ports if p.device.upper() not in before]
        preferred = [p for p in new_ports if is_espressif_port(p)]
        if len(preferred) == 1:
            return preferred[0].device
        if not preferred:
            preferred = [p for p in ports if is_espressif_port(p)]
            if len(preferred) == 1:
                return preferred[0].device
        time.sleep(0.5)
    return None


def choose_port() -> Optional[str]:
    ports_before = available_ports()
    before = {port.device.upper() for port in ports_before}

    print("\n[Upload] The normal USB HID firmware intentionally has no COM port.")
    print("A temporary COM port appears only in the ESP32-S3 ROM download mode.")
    print("\nEnter download mode now:")
    print("  1. Keep the CrowPanel connected with the included data-capable USB lead.")
    print("  2. Hold BOOT.")
    print("  3. Press and release RESET while still holding BOOT.")
    print("  4. Release BOOT.")
    try:
        input("\nPress Enter after completing BOOT + RESET: ")
    except (KeyboardInterrupt, EOFError):
        print()
        return None

    print("[Upload] Looking for the ESP32-S3 download port ...")
    detected = auto_detect_download_port(before)
    if detected:
        print(f"[Upload] Automatically selected {detected}.")
        return detected

    while True:
        ports = available_ports()
        print_ports(ports)
        try:
            choice = input(
                "\nSelect port number, enter COM name, R to rescan, "
                "B for BOOT instructions, or Q to quit: "
            ).strip()
        except (KeyboardInterrupt, EOFError):
            print()
            return None
        if not choice or choice.lower() == "r":
            continue
        if choice.lower() == "b":
            print("Hold BOOT, tap RESET, release BOOT, then choose R to rescan.")
            continue
        if choice.lower() == "q":
            return None
        if choice.isdigit() and 1 <= int(choice) <= len(ports):
            return ports[int(choice) - 1].device
        if choice.upper().startswith("COM") and choice[3:].isdigit():
            return choice.upper()
        print("Invalid selection. Try a COM port such as COM7.")


def build_command() -> list[str]:
    return [str(PIO), "run", "-e", ENVIRONMENT]


def main() -> int:
    parser = argparse.ArgumentParser(description="Build / flash CrowPanel Rotary Buddy USB HID")
    parser.add_argument("command", choices=("build", "upload", "web"))
    parser.add_argument("--port", help="Optional Windows COM port, for example COM7")
    args = parser.parse_args()

    if os.name != "nt":
        print("This helper is for Windows. On other systems use PlatformIO directly.")
        return 2

    if not prepare_environment():
        return 1

    print("\n[Build] Compiling firmware before touching the device ...")
    code = run_command(build_command())
    if code != 0:
        print("\nBuild failed. Nothing was flashed and no COM port is required yet.")
        print("Review the compiler error above, then run the batch file again.")
        return code

    if args.command == "build":
        print("\nSuccess. Firmware binary is under .pio/build/crowpanel-rotary/.")
        return 0
    if args.command == "web":
        print("[Web export] Installing the pinned Espressif merge utility ...")
        if run_command([str(PYTHON), "-m", "pip", "install", "--disable-pip-version-check",
                        "esptool==4.8.1"]) != 0:
            return 1
        print("[Web export] Exporting browser install/update images ...")
        return run_command([str(PYTHON), str(ROOT / "tools" / "export_web.py")])


    port = args.port or choose_port()
    if not port:
        print("Upload cancelled.")
        return 1

    print(f"\n[Upload] Flashing the already-built firmware to {port} ...")
    upload = build_command() + ["-t", "upload", "--upload-port", port]
    code = run_command(upload)
    if code == 0:
        print("\nSuccess. Tap RESET once or reconnect USB if the application does not start.")
        print("Windows should detect a USB HID keyboard/media-control device.")
        print("The temporary bootloader COM port normally disappears after startup.")
    else:
        print("\nUpload failed. The firmware build itself was successful.")
        print("Close serial monitors, repeat BOOT + RESET, and run FLASH_WINDOWS.bat again.")
    return code


if __name__ == "__main__":
    sys.exit(main())
