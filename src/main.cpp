#include <Arduino.h>
#include <USB.h>
#include <USBHID.h>
#include <USBHIDConsumerControl.h>
#include <Preferences.h>
#include <LovyanGFX.hpp>
#include <driver/gpio.h>

#include "Config.h"
#include "CST816Touch.h"
#include "TouchGesture.h"
#include "RoundDisplay.h"
#include "VolumeScaler.h"
#include "MediaLogic.h"
#include "BuddyBrain.h"
#include "BuddyGraphics.h"

// Rotary Buddy: two pages, standard USB media HID on every host.
// No host shortcuts, Wi-Fi, Bluetooth, native companion application, or audio.
static USBHID hid;
static USBHIDConsumerControl consumer;
static RoundDisplay display;
static lgfx::LGFX_Sprite framebuffer(&display);
static bool framebufferReady = false;
static CST816Touch touch;
static TouchGesture gesture;
static VolumeScaler volumeScaler(Config::VOLUME_SPEED_DIVISOR);
static VolumeFeedback volumeFeedback;
static BuddyBrain buddy;
static Preferences preferences;

enum class Screen : uint8_t { Media, Buddy };
static Screen screen = Screen::Media;
static bool touchReady = false;
static bool ignoreTouchUntilRelease = false;
static bool switchRaw = false;
static bool switchStable = false;
static bool longPressHandled = false;
static bool volumeKeyDown = false;
static bool usbConnected = false;
static uint32_t switchEdgeAt = 0;
static uint32_t switchPressedAt = 0;
static uint32_t lastTouchPollAt = 0;
static uint32_t touchStartedAt = 0;
static uint32_t lastPageAt = 0;
static uint32_t lastTapAt = 0;
static uint32_t lastUserActionAt = 0;
static uint32_t lastDetentAt = 0;
static uint32_t keyPressedAt = 0;
static uint32_t lastVolumeReportAt = 0;
static uint32_t lastFrameAt = 0;
static uint32_t feedbackAt = 0;
static uint32_t pageChangedAt = 0;
static uint32_t lastDisconnectedCheckAt = 0;
static VolumeQueue queueVolume(Config::MAX_PENDING_VOLUME_PULSES);
static uint8_t feedbackKind = 0; // 1 prev, 2 next, 3 play/pause, 4 mute
static uint8_t brightness = Config::DEFAULT_BRIGHTNESS;

// Interrupt sampling ensures fast encoder edges are not lost while the 240x240
// framebuffer is pushed over SPI. Signed quadrature polarity follows encoder wiring.
static volatile uint8_t encoderPrevious = 0;
static volatile int16_t encoderDelta = 0;
static int encoderCarry = 0;

static int8_t IRAM_ATTR transition(uint8_t code) {
    switch (code) {
        case 0x02: case 0x04: case 0x0B: case 0x0D: return 1;
        case 0x01: case 0x07: case 0x08: case 0x0E: return -1;
        default: return 0;
    }
}

static void IRAM_ATTR encoderInterrupt() {
    const uint8_t level = (static_cast<uint8_t>(gpio_get_level(static_cast<gpio_num_t>(Config::ENCODER_A))) << 1) |
                          static_cast<uint8_t>(gpio_get_level(static_cast<gpio_num_t>(Config::ENCODER_B)));
    const uint8_t code = (encoderPrevious << 2) | level;
    encoderPrevious = level;
    int next = static_cast<int>(encoderDelta) + transition(code);
    if (next > 800) next = 800;
    if (next < -800) next = -800;
    encoderDelta = static_cast<int16_t>(next);
}

static void markActivity(uint32_t now, bool playful = false) {
    lastUserActionAt = now;
    buddy.activity(now, playful);
}

static bool mediaKey(uint16_t usage) {
    if (!hid.ready()) return false;
    if (!consumer.press(usage)) return false;
    delay(12);
    return consumer.release() != 0;
}

static void navigate(uint32_t now, bool manual) {
    if (manual && (now - lastPageAt < Config::TOUCH_NAV_COOLDOWN_MS)) return;
    lastPageAt = now;
    pageChangedAt = now;
    feedbackKind = 0;
    screen = screen == Screen::Media ? Screen::Buddy : Screen::Media;
    if (manual) markActivity(now, screen == Screen::Buddy);
}

static void handleEncoder(uint32_t now) {
    noInterrupts();
    const int edges = encoderDelta;
    encoderDelta = 0;
    interrupts();
    encoderCarry += edges;

    int steps = 0;
    while (encoderCarry >= Config::ENCODER_EDGES_PER_STEP) {
        encoderCarry -= Config::ENCODER_EDGES_PER_STEP;
        ++steps;
    }
    while (encoderCarry <= -Config::ENCODER_EDGES_PER_STEP) {
        encoderCarry += Config::ENCODER_EDGES_PER_STEP;
        --steps;
    }
    if (!steps) return;

    const int direction = steps * Config::ENCODER_DIRECTION;
    const bool fast = (steps > 1 || steps < -1) ||
                      (lastDetentAt != 0 && (now - lastDetentAt) < Config::FAST_TURN_MS);
    lastDetentAt = now;
    const int pulses = fast ? Config::FAST_VOLUME_PULSES : Config::VOLUME_PULSES_PER_DETENT;
    const int scaled = volumeScaler.apply(direction * pulses);
    markActivity(now);
    if (!scaled) return;
    queueVolume.add(scaled);
    volumeFeedback.turn(scaled, now);
}

static void flushVolume(uint32_t now) {
    if (!hid.ready()) {
        if (volumeKeyDown) consumer.release();
        volumeKeyDown = false;
        queueVolume.reset();
        volumeScaler.reset();
        return;
    }
    if (volumeKeyDown) {
        if (now - keyPressedAt >= Config::VOLUME_KEY_HOLD_MS) {
            if (consumer.release()) {
                volumeKeyDown = false;
                lastVolumeReportAt = now;
            }
        }
        return;
    }
    if (queueVolume.pending() == 0 || now - lastVolumeReportAt < Config::VOLUME_EVENT_GAP_MS)
        return;
    const int direction = queueVolume.direction();
    // Do not discard a queued command if the USB endpoint is temporarily busy.
    if (consumer.press(direction > 0 ? CONSUMER_CONTROL_VOLUME_INCREMENT :
                                       CONSUMER_CONTROL_VOLUME_DECREMENT)) {
        volumeKeyDown = true;
        keyPressedAt = now;
        queueVolume.sent();
    }
}

static void runMediaButton(MediaButton button, uint32_t now) {
    uint16_t usage = 0;
    switch (button) {
        case MediaButton::Previous:
            usage = CONSUMER_CONTROL_SCAN_PREVIOUS;
            feedbackKind = 1;
            break;
        case MediaButton::PlayPause:
            usage = CONSUMER_CONTROL_PLAY_PAUSE;
            feedbackKind = 3;
            break;
        case MediaButton::Next:
            usage = CONSUMER_CONTROL_SCAN_NEXT;
            feedbackKind = 2;
            break;
        case MediaButton::Mute:
            usage = CONSUMER_CONTROL_MUTE;
            feedbackKind = 4;
            break;
        default: return;
    }
    markActivity(now);
    // Never overlap a pending volume press with a different media usage.
    if (volumeKeyDown) {
        if (consumer.release()) {
            volumeKeyDown = false;
            lastVolumeReportAt = now;
        } else {
            feedbackKind = 0;
            return;
        }
    }
    if (!mediaKey(usage)) feedbackKind = 0;
    feedbackAt = now;
}

static void handleSwitch(uint32_t now) {
    const bool pressed = digitalRead(Config::ENCODER_SWITCH) == LOW;
    if (pressed != switchRaw) {
        switchRaw = pressed;
        switchEdgeAt = now;
    }
    if ((now - switchEdgeAt >= Config::SWITCH_DEBOUNCE_MS) &&
        pressed != switchStable) {
        switchStable = pressed;
        if (switchStable) {
            switchPressedAt = now;
            longPressHandled = false;
            markActivity(now);
        } else if (!longPressHandled) {
            runMediaButton(MediaButton::PlayPause, now);
        }
    }
    if (switchStable && !longPressHandled &&
        (now - switchPressedAt >= Config::HOLD_SWITCH_PAGE_MS)) {
        longPressHandled = true;
        // A long hold does nothing. Page changes are swipe-only.
        // The release must not send Play/Pause after a long hold.
    }
}

static void handleTouch(uint32_t now) {
    if (!touchReady || now - lastTouchPollAt < Config::TOUCH_POLL_MS) return;
    lastTouchPollAt = now;
    TouchSample point;
    if (!touch.read(point)) return;

    if (ignoreTouchUntilRelease) {
        if (point.released || !point.pressed) ignoreTouchUntilRelease = false;
        return;
    }
    const bool previouslyActive = gesture.active();
    if (!previouslyActive && point.pressed) touchStartedAt = now;
    if (!previouslyActive && !point.pressed && !point.released) return;

    const GestureResult event = gesture.consume(
        point.pressed, point.released, point.gesture, point.x, point.y, now,
        Config::TOUCH_MIN_TAP_MS, Config::TOUCH_MAX_TAP_MS,
        Config::TOUCH_TAP_SLOP_PX, Config::TOUCH_SWIPE_MIN_PX);

    if (event.type == GestureType::SwipeLeft || event.type == GestureType::SwipeRight) {
        // Only horizontal swipes change pages, never a tap or a vertical gesture.
        navigate(now, true);
    } else if (event.type == GestureType::PetDown) {
        if (screen == Screen::Buddy) buddy.pet(now);
        lastUserActionAt = now; // No multimedia command is sent for petting.
    } else if (event.type == GestureType::Tap && now - lastTapAt >= 110) {
        lastTapAt = now;
        if (screen == Screen::Buddy) {
            markActivity(now, true); // boop: a new expression, no host key
        } else {
            // The volume animation temporarily hides media buttons. The first
            // tap dismisses it instead of activating a hidden control.
            if (volumeFeedback.visible(now, Config::VOLUME_OVERLAY_MS)) {
                volumeFeedback.dismiss();
                return;
            }
            const MediaButton start = mediaHitTest(event.startX, event.startY);
            const MediaButton end = mediaHitTest(event.endX, event.endY);
            if (start != MediaButton::None && start == end)
                runMediaButton(start, now);
        }
    }
    if (gesture.active() && now - touchStartedAt > Config::TOUCH_MAX_TRACK_MS) {
        gesture.reset();
        ignoreTouchUntilRelease = true;
    }
}

static void drawFrame(uint32_t now) {
    const bool showVolume = volumeFeedback.visible(now, Config::VOLUME_OVERLAY_MS);
    const bool showFeedback = (now - feedbackAt) < Config::FEEDBACK_MS;
    const int kind = showFeedback ? feedbackKind : 0;
    const uint32_t since = now - feedbackAt;
    if (screen == Screen::Media) {
        if (framebufferReady) {
            framebuffer.startWrite();
            Art::media(framebuffer, now, volumeFeedback.direction(), showVolume,
                       kind, since, usbConnected);
            Art::transition(framebuffer, now - pageChangedAt, Config::TRANSITION_MS, false);
            framebuffer.endWrite();
            framebuffer.pushSprite(0, 0);
        } else {
            display.startWrite();
            Art::media(display, now, volumeFeedback.direction(), showVolume,
                       kind, since, usbConnected);
            Art::transition(display, now - pageChangedAt, Config::TRANSITION_MS, false);
            display.endWrite();
        }
    } else {
        if (framebufferReady) {
            framebuffer.startWrite();
            Art::buddy(framebuffer, now, buddy.mood(), buddy.elapsed(now), buddy.quip(),
                       showVolume, volumeFeedback.direction(), usbConnected);
            Art::transition(framebuffer, now - pageChangedAt, Config::TRANSITION_MS, true);
            framebuffer.endWrite();
            framebuffer.pushSprite(0, 0);
        } else {
            display.startWrite();
            Art::buddy(display, now, buddy.mood(), buddy.elapsed(now), buddy.quip(),
                       showVolume, volumeFeedback.direction(), usbConnected);
            Art::transition(display, now - pageChangedAt, Config::TRANSITION_MS, true);
            display.endWrite();
        }
    }
}

static void startupAnimation() {
    const uint32_t start = millis();
    uint32_t previous = 0;
    while (millis() - start < Config::BOOT_ANIMATION_MS) {
        const uint32_t now = millis();
        if (now - previous >= 50) {
            previous = now;
            const uint32_t t = now - start;
            const float progress = Art::easeOut(static_cast<float>(t) /
                                                   Config::BOOT_ANIMATION_MS);
            if (framebufferReady) {
                Art::base(framebuffer, now);
                Art::arc(framebuffer, 120, 120, 98, 180, 180 + 360 * progress, Art::CYAN, 3);
                if (t > 350) {
                    Art::text(framebuffer, "R O T A R Y", 120, 66, 2, Art::WHITE);
                    Art::text(framebuffer, "BUDDY", 120, 104, 4, Art::CYAN);
                    if (t > 650) {
                        Art::arc(framebuffer, 90, 154, 15, 193, 347, Art::SKY, 4);
                        Art::arc(framebuffer, 150, 154, 15, 193, 347, Art::SKY, 4);
                        Art::spark(framebuffer, 184, 144, 6, Art::GOLD);
                    }
                }
                framebuffer.pushSprite(0, 0);
            } else {
                Art::base(display, now);
                Art::arc(display, 120, 120, 98, 180, 180 + 360 * progress, Art::CYAN, 3);
                if (t > 350) {
                    Art::text(display, "R O T A R Y", 120, 66, 2, Art::WHITE);
                    Art::text(display, "BUDDY", 120, 104, 4, Art::CYAN);
                }
            }
        }
        delay(1);
    }
}

static void setBacklight(uint8_t percent) {
    ledcWrite(0, (static_cast<uint32_t>(percent) * 255u) / 100u);
}

void setup() {
    pinMode(Config::SCREEN_POWER, OUTPUT);
    digitalWrite(Config::SCREEN_POWER, HIGH);
    pinMode(Config::AUX_POWER, OUTPUT);
    digitalWrite(Config::AUX_POWER, HIGH);
    pinMode(Config::POWER_LED, OUTPUT);
    digitalWrite(Config::POWER_LED, LOW);
    delay(60);

    pinMode(Config::LCD_BACKLIGHT, OUTPUT);
    digitalWrite(Config::LCD_BACKLIGHT, LOW);
    display.init();
    display.setRotation(Config::SCREEN_ROTATION);
    display.setColorDepth(16);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(Config::LCD_BACKLIGHT, 0);
    const bool prefs = preferences.begin("rotarybuddy", false);
    if (prefs) {
        const uint8_t saved = preferences.getUChar("brightness", Config::DEFAULT_BRIGHTNESS);
        if (saved >= 25 && saved <= 100) brightness = saved;
    }
    setBacklight(brightness);

    framebuffer.setColorDepth(16);
    framebufferReady = Config::USE_FRAMEBUFFER && framebuffer.createSprite(240, 240) != nullptr;

    pinMode(Config::ENCODER_A, INPUT_PULLUP);
    pinMode(Config::ENCODER_B, INPUT_PULLUP);
    pinMode(Config::ENCODER_SWITCH, INPUT_PULLUP);
    encoderPrevious = (static_cast<uint8_t>(digitalRead(Config::ENCODER_A)) << 1) |
                       static_cast<uint8_t>(digitalRead(Config::ENCODER_B));
    attachInterrupt(digitalPinToInterrupt(Config::ENCODER_A), encoderInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Config::ENCODER_B), encoderInterrupt, CHANGE);

    // Polling I2C touch driver, matching Elecrow CST816D wiring.
    touchReady = touch.begin();
    USB.manufacturerName("lepczynski-cloud");
    USB.productName("CrowPanel Rotary Buddy");
    consumer.begin();
    USB.begin();
    usbConnected = hid.ready();

    const uint32_t now = millis();
    buddy.begin(now);
    lastUserActionAt = now;
    startupAnimation();
    const uint32_t ready = millis();
    lastUserActionAt = ready;
    buddy.activity(ready);
    pageChangedAt = ready - Config::TRANSITION_MS;
    drawFrame(ready);
}

void loop() {
    const uint32_t now = millis();
    handleEncoder(now);
    handleSwitch(now);
    handleTouch(now);
    flushVolume(now);
    buddy.update(now, Config::SLEEP_AFTER_MS);

    if (shouldAutoBuddy(now, lastUserActionAt, screen == Screen::Media,
                        Config::AUTO_BUDDY_AFTER_MS)) {
        navigate(now, false); // Does not reset inactivity timer; sleep remains reachable.
    }
    if (now - lastDisconnectedCheckAt >= 500) {
        lastDisconnectedCheckAt = now;
        usbConnected = hid.ready();
    }
    if (now - lastFrameAt >= Config::FRAME_INTERVAL_MS) {
        lastFrameAt = now;
        drawFrame(now);
    }
    delay(1);
}
