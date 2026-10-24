#pragma once
#include <stdint.h>

// CrowPanel 1.28-inch HMI ESP32-S3 rotary display (Elecrow DHE38128D).
// Hardware pins follow Elecrow board wiring.
namespace Config {
constexpr int LCD_SCLK = 10;
constexpr int LCD_MOSI = 11;
constexpr int LCD_DC = 3;
constexpr int LCD_CS = 9;
constexpr int LCD_RESET = 14;
constexpr int LCD_BACKLIGHT = 46;
constexpr int SCREEN_POWER = 1;
constexpr int AUX_POWER = 2;
constexpr int POWER_LED = 40;
constexpr int TOUCH_SDA = 6;
constexpr int TOUCH_SCL = 7;
constexpr int TOUCH_IRQ = 5;
constexpr int TOUCH_RESET = 13;
constexpr unsigned long TOUCH_I2C_HZ = 100000;
constexpr int ENCODER_A = 45;
constexpr int ENCODER_B = 42;
constexpr int ENCODER_SWITCH = 41;

// Adjust only if your hardware has a different touch orientation.
constexpr int SCREEN_ROTATION = 0;
constexpr bool TOUCH_SWAP_XY = false;
constexpr bool TOUCH_INVERT_X = false;
constexpr bool TOUCH_INVERT_Y = false;

// Volume speed: 2 = slower, 1 = faster; tune to taste.
// Both pages use the knob for host volume and the button for Play/Pause.
constexpr int ENCODER_DIRECTION = 1;
constexpr int ENCODER_EDGES_PER_STEP = 4;
constexpr int VOLUME_PULSES_PER_DETENT = 3;
constexpr int FAST_VOLUME_PULSES = 5;
constexpr int VOLUME_SPEED_DIVISOR = 2;
constexpr unsigned long FAST_TURN_MS = 100;
constexpr unsigned long VOLUME_EVENT_GAP_MS = 6;
constexpr unsigned long VOLUME_KEY_HOLD_MS = 12;
constexpr int MAX_PENDING_VOLUME_PULSES = 120;

// Volume display is ONLY an estimate, not a reading from the host computer.
// HID volume up/down cannot query the real volume percentage.
constexpr int VOLUME_INDICATOR_INITIAL_PERCENT = 50;
constexpr int VOLUME_INDICATOR_PERCENT_PER_REPORT = 2;
constexpr unsigned long VOLUME_OVERLAY_MS = 2300;

// CST816D is read by I2C polling so an IRQ polarity mismatch cannot block touch.
constexpr unsigned long TOUCH_POLL_MS = 12;
constexpr unsigned long TOUCH_MAX_TRACK_MS = 1300;
constexpr unsigned long TOUCH_MIN_TAP_MS = 12;
constexpr unsigned long TOUCH_MAX_TAP_MS = 500;
constexpr int TOUCH_TAP_SLOP_PX = 19;
constexpr int TOUCH_SWIPE_MIN_PX = 43;
constexpr unsigned long TOUCH_NAV_COOLDOWN_MS = 320;
constexpr unsigned long SWITCH_DEBOUNCE_MS = 28;
constexpr unsigned long HOLD_SWITCH_PAGE_MS = 650;

// Timeouts measured since the most recent user interaction.
constexpr unsigned long AUTO_BUDDY_AFTER_MS = 30000;  // Media -> Buddy
constexpr unsigned long SLEEP_AFTER_MS = 300000;      // Buddy starts dreaming
constexpr unsigned long FRAME_INTERVAL_MS = 45;      // ~22 FPS, non-blocking
constexpr unsigned long TRANSITION_MS = 340;
constexpr unsigned long BOOT_ANIMATION_MS = 1550;
constexpr unsigned long FEEDBACK_MS = 950;

// 0 disables saving to flash. Brightness is persisted only if changed.
constexpr uint8_t DEFAULT_BRIGHTNESS = 85;
constexpr bool USE_FRAMEBUFFER = true; // fallback is direct drawing if allocation fails
}
