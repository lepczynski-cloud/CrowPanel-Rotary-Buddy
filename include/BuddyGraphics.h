#pragma once
#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "BuddyBrain.h"
#include "MediaLogic.h"

// All artwork is drawn from original vector primitives. No font downloads,
// GIFs, assets, spritesheets or proprietary robot face animations are used.
namespace Art {
constexpr float PI_F = 3.14159265358979323846f;
constexpr float DEG = 0.017453292519943295f;
constexpr uint16_t BG = 0x0046;        // deep navy
constexpr uint16_t EDGE = 0x110E;      // glass bezel
constexpr uint16_t INNER = 0x08EA;     // faceplate
constexpr uint16_t PANEL = 0x114F;
constexpr uint16_t PNL_LIT = 0x1A32;
constexpr uint16_t CYAN = 0x56FF;
constexpr uint16_t SKY = 0x3DDF;
constexpr uint16_t VIOLET = 0xA43D;
constexpr uint16_t ROSE = 0xF3BC;
constexpr uint16_t GOLD = 0xFF2C;
constexpr uint16_t WHITE = 0xF7FF;
constexpr uint16_t MUTED = 0x7C70;
constexpr uint16_t DIM = 0x39CC;
constexpr uint16_t GREEN = 0x5F77;

inline int ix(float n) { return static_cast<int>(n + (n >= 0 ? 0.5f : -0.5f)); }
inline float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
inline float wave(uint32_t ms, float speed, float phase = 0) { return sinf(static_cast<float>(ms) * speed + phase); }
inline float easeOut(float t) { t = clampf(t, 0, 1); return 1.0f - (1.0f - t) * (1.0f - t); }

template <typename G>
void text(G &g, const char *str, int x, int y, int font, uint16_t color, int scale = 1) {
    g.setTextDatum(middle_center);
    g.setTextColor(color);
    g.setTextFont(font);
    g.setTextSize(scale);
    g.drawString(str, x, y);
}

template <typename G>
void arc(G &g, int cx, int cy, float radius, float from, float to, uint16_t color, int thick = 1) {
    const int segments = static_cast<int>((to - from) / 5.0f) + 1;
    for (int i = 0; i < segments; ++i) {
        const float a = (from + (to - from) * static_cast<float>(i) / segments) * DEG;
        const float b = (from + (to - from) * static_cast<float>(i + 1) / segments) * DEG;
        const int x1 = cx + ix(cosf(a) * radius);
        const int y1 = cy + ix(sinf(a) * radius);
        const int x2 = cx + ix(cosf(b) * radius);
        const int y2 = cy + ix(sinf(b) * radius);
        g.drawLine(x1, y1, x2, y2, color);
        if (thick >= 2) g.drawLine(x1 - 1, y1, x2 - 1, y2, color);
        if (thick >= 3) g.drawLine(x1, y1 - 1, x2, y2 - 1, color);
        if (thick >= 4) g.drawLine(x1 + 1, y1, x2 + 1, y2, color);
    }
}

template <typename G>
void spark(G &g, int x, int y, int size, uint16_t color) {
    if (size < 1) return;
    g.drawLine(x - size, y, x + size, y, color);
    g.drawLine(x, y - size, x, y + size, color);
    if (size >= 5) {
        g.drawLine(x - size / 2, y - size / 2, x + size / 2, y + size / 2, color);
        g.drawLine(x + size / 2, y - size / 2, x - size / 2, y + size / 2, color);
    }
}

template <typename G>
void heart(G &g, int x, int y, int scale, uint16_t color) {
    const int r = 5 * scale;
    g.fillCircle(x - r, y - r / 2, r, color);
    g.fillCircle(x + r, y - r / 2, r, color);
    g.fillTriangle(x - r * 2, y, x + r * 2, y, x, y + r * 2, color);
}

template <typename G>
void star(G &g, int x, int y, int r, uint16_t color) {
    g.fillTriangle(x, y - r, x - r / 3, y, x + r / 3, y, color);
    g.fillTriangle(x, y + r, x - r / 3, y, x + r / 3, y, color);
    g.fillTriangle(x - r, y, x, y - r / 3, x, y + r / 3, color);
    g.fillTriangle(x + r, y, x, y - r / 3, x, y + r / 3, color);
}

template <typename G>
void base(G &g, uint32_t now) {
    g.fillScreen(BG);
    g.drawCircle(120, 120, 119, EDGE);
    g.drawCircle(120, 120, 116, PANEL);
    g.drawCircle(120, 120, 113, EDGE);
    const float step = static_cast<float>(now % 5000) * 0.0010f;
    const int x = 120 + ix(107 * cosf(step));
    const int y = 120 + ix(107 * sinf(step));
    g.fillCircle(x, y, 2, CYAN);
    g.fillCircle(38, 45, 1, SKY);
    g.fillCircle(205, 57, 1, VIOLET);
    g.fillCircle(28, 178, 1, GOLD);
}

template <typename G>
void pageDots(G &g, int page) {
    g.fillCircle(114, 224, page == 0 ? 4 : 2, page == 0 ? CYAN : DIM);
    g.fillCircle(128, 224, page == 1 ? 4 : 2, page == 1 ? CYAN : DIM);
}

template <typename G>
void mediaGlyph(G &g, int id, int cx, int cy, uint16_t color, bool muted = false) {
    if (id == 0) { // previous
        g.fillRect(cx - 11, cy - 12, 4, 24, color);
        g.fillTriangle(cx - 6, cy, cx + 12, cy - 13, cx + 12, cy + 13, color);
    } else if (id == 1) { // combined play/pause (USB reports a toggle)
        g.fillTriangle(cx - 13, cy - 16, cx - 13, cy + 16, cx + 3, cy, color);
        g.fillRoundRect(cx + 9, cy - 13, 4, 26, 2, color);
        g.fillRoundRect(cx + 17, cy - 13, 4, 26, 2, color);
    } else if (id == 2) { // next
        g.fillRect(cx + 7, cy - 12, 4, 24, color);
        g.fillTriangle(cx + 6, cy, cx - 12, cy - 13, cx - 12, cy + 13, color);
    } else { // Speaker: waves while active, red slash while muted.
        g.fillRect(cx - 16, cy - 6, 9, 12, color);
        g.fillTriangle(cx - 9, cy - 6, cx + 1, cy - 13, cx + 1, cy + 13, color);
        if (muted) {
            g.drawLine(cx + 6, cy - 13, cx + 20, cy + 14, ROSE);
            g.drawLine(cx + 8, cy - 13, cx + 22, cy + 14, ROSE);
        } else {
            arc(g, cx + 2, cy, 10, -62, 62, CYAN, 2);
            arc(g, cx + 2, cy, 17, -60, 60, SKY, 2);
        }
    }
}

template <typename G>
void skipParticles(G &g, uint32_t elapsed, int direction) {
    if (elapsed >= 950) return;
    const float t = easeOut(static_cast<float>(elapsed) / 950.0f);
    const int origin = direction < 0 ? 48 : 192;
    for (int i = 0; i < 11; ++i) {
        const float a = ((i * 49 + (direction < 0 ? 145 : -35)) % 360) * DEG;
        const float distance = 12.0f + t * (44 + (i % 4) * 8);
        const int x = origin + ix(cosf(a) * distance);
        const int y = 121 + ix(sinf(a) * distance);
        if ((x - 120) * (x - 120) + (y - 120) * (y - 120) < 105 * 105)
            spark(g, x, y, i % 4 == 0 ? 5 : 2, i % 3 ? CYAN : GOLD);
    }
    arc(g, 120, 122, 91, direction < 0 ? 170 - 70 * t : -10,
        direction < 0 ? 240 - 70 * t : 60 + 70 * t, SKY, 3);
}

template <typename G>
void drawVolumeRing(G &g, int percent, uint32_t now, bool vivid) {
    arc(g, 120, 120, 104, 140, 400, EDGE, 3);
    const float end = 140.0f + 260.0f * static_cast<float>(percent) / 100.0f;
    if (percent > 0) arc(g, 120, 120, 104, 140, end, vivid ? CYAN : SKY, vivid ? 4 : 2);
    const float theta = end * DEG;
    const int x = 120 + ix(cosf(theta) * 104);
    const int y = 120 + ix(sinf(theta) * 104);
    g.fillCircle(x, y, vivid ? 5 : 3, vivid ? WHITE : SKY);
    if (vivid) {
        const float offset = wave(now, 0.009f);
        g.drawCircle(x, y, 7 + ix(fabsf(offset) * 3), CYAN);
    }
}

template <typename G>
void media(G &g, uint32_t now, int volumePct, bool showVolume,
           int feedbackKind, uint32_t feedbackElapsed, bool usbConnected, bool mutedLocally) {
    base(g, now);
    text(g, "R O T A R Y   B U D D Y", 120, 28, 1, MUTED);
    text(g, "MEDIA", 120, 51, 4, WHITE);
    g.fillCircle(197, 46, 3, usbConnected ? GREEN : ROSE);
    drawVolumeRing(g, volumePct, now, showVolume);
    if (showVolume) {
        g.fillCircle(120, 119, 83, BG);
        g.drawCircle(120, 119, 83, PANEL);
        char label[12];
        snprintf(label, sizeof(label), "%d%%", volumePct);
        text(g, "VOLUME", 120, 83, 2, MUTED);
        text(g, label, 120, 126, 4, WHITE, 2);
        text(g, "ESTIMATED", 120, 165, 1, SKY);
    } else {
        const int locations[4][2] = {{47, 119}, {120, 119}, {193, 119}, {120, 188}};
        for (int i = 0; i < 4; ++i) {
            const int radius = i == 1 ? 37 : (i == 3 ? 30 : 31);
            const uint16_t fill = i == 1 ? CYAN : PANEL;
            g.fillCircle(locations[i][0], locations[i][1], radius, fill);
            g.drawCircle(locations[i][0], locations[i][1], radius, i == 1 ? SKY : DIM);
            mediaGlyph(g, i, locations[i][0], locations[i][1], i == 1 ? BG : WHITE, mutedLocally);
        }
        text(g, "PREV", 47, 83, 1, MUTED);
        text(g, "NEXT", 193, 83, 1, MUTED);
        text(g, mutedLocally ? "MUTED" : "SOUND ON", 120, 208, 1, mutedLocally ? ROSE : SKY);
    }
    if (feedbackKind == 1 || feedbackKind == 2) {
        skipParticles(g, feedbackElapsed, feedbackKind == 1 ? -1 : 1);
    } else if (feedbackKind == 3 && feedbackElapsed < 820) {
        const float t = static_cast<float>(feedbackElapsed) / 820.0f;
        arc(g, 120, 121, 38 + t * 44, 0, 350, VIOLET, 3);
        spark(g, 161 + ix(t * 28), 80, 5, GOLD);
    } else if (feedbackKind == 4 && feedbackElapsed < 720) {
        for (int i = 0; i < 7; ++i) {
            const float a = (i * 51 + feedbackElapsed / 18) * DEG;
            spark(g, 120 + ix(cosf(a) * 53), 184 + ix(sinf(a) * 25), 3, ROSE);
        }
    }
    pageDots(g, 0);
}

// Eye style and expression calculation are intentionally data-driven so a
// single small renderer can generate dozens of noticeably different moods.
struct Face {
    int gazeX = 0;
    int gazeY = 0;
    int eyeWidth = 29;
    int eyeHeight = 33;
    int eyebrow = 0; // -1 anger, +1 curiosity
    int mouth = 0;   // 0 tiny, 1 smile, 2 open, 3 pout, 4 laughing, 5 tongue
    int eyeStyle = 0; // 0 normal, 1 closed, 2 smiles, 3 heart, 4 star, 5 spiral
    bool winkLeft = false;
    bool winkRight = false;
    bool cheeks = false;
    bool tears = false;
    bool blush = false;
    int tilt = 0;
    uint16_t eyeColor = CYAN;
};

inline Face faceFor(Mood mood, uint32_t elapsed, uint32_t now) {
    Face f;
    switch (mood) {
        case Mood::Blink: f.eyeStyle = (elapsed % 760 < 250) ? 1 : 0; break;
        case Mood::DoubleBlink: f.eyeStyle = (elapsed % 600 < 135 || (elapsed % 600 > 270 && elapsed % 600 < 400)) ? 1 : 0; break;
        case Mood::LookLeft: f.gazeX = -10; break;
        case Mood::LookRight: f.gazeX = 10; break;
        case Mood::LookUp: f.gazeY = -10; break;
        case Mood::LookDown: f.gazeY = 9; break;
        case Mood::WinkLeft: f.winkLeft = true; f.mouth = 1; break;
        case Mood::WinkRight: f.winkRight = true; f.mouth = 1; break;
        case Mood::Happy: f.eyeStyle = 2; f.mouth = 1; break;
        case Mood::Laugh: f.eyeStyle = 2; f.mouth = 4; f.tilt = ix(wave(now, .020f) * 4); break;
        case Mood::Blush: f.eyeStyle = 2; f.mouth = 1; f.blush = true; break;
        case Mood::Hearts: f.eyeStyle = 3; f.mouth = 1; f.eyeColor = ROSE; break;
        case Mood::StarEyes: f.eyeStyle = 4; f.mouth = 2; f.eyeColor = GOLD; break;
        case Mood::Sparkles: f.eyeStyle = 2; f.mouth = 1; break;
        case Mood::Surprise: f.eyeWidth = 38; f.eyeHeight = 40; f.mouth = 2; break;
        case Mood::Confused: f.eyebrow = 1; f.gazeX = 6; f.mouth = 3; break;
        case Mood::Thinking: f.eyebrow = 1; f.gazeX = -7; f.gazeY = -8; break;
        case Mood::SideEye: f.gazeX = -14; f.eyebrow = -1; break;
        case Mood::Pout: f.mouth = 3; f.gazeY = 5; break;
        case Mood::Grumpy: f.eyebrow = -1; f.mouth = 3; break;
        case Mood::Mischief: f.winkRight = true; f.mouth = 1; f.eyebrow = -1; break;
        case Mood::Tongue: f.mouth = 5; f.winkLeft = true; break;
        case Mood::Giggle: f.eyeStyle = 2; f.mouth = 4; f.tilt = ix(wave(now, .027f) * 3); break;
        case Mood::Party: f.eyeStyle = 4; f.mouth = 4; f.tilt = ix(wave(now, .015f) * 9); f.eyeColor = GOLD; break;
        case Mood::Bounce: f.eyeStyle = 2; f.mouth = 1; f.tilt = ix(wave(now, .013f) * 11); break;
        case Mood::Dizzy: f.eyeStyle = 5; f.mouth = 2; break;
        case Mood::Peek: f.winkLeft = true; f.gazeX = 12; break;
        case Mood::Shy: f.gazeY = 12; f.eyeHeight = 23; f.blush = true; break;
        case Mood::Yawn: f.eyeStyle = 1; f.mouth = 2; break;
        case Mood::Sing: f.mouth = 2; f.gazeY = -4; f.eyeStyle = 2; break;
        case Mood::Tears: f.mouth = 3; f.tears = true; break;
        case Mood::Sleepy: f.eyeStyle = 1; f.mouth = 2; break;
        case Mood::RollEyes: f.gazeX = ix(9 * cosf(elapsed * .011f)); f.gazeY = ix(9 * sinf(elapsed * .011f)); break;
        case Mood::Idea: f.eyeWidth = 35; f.eyeHeight = 38; f.mouth = 1; break;
        case Mood::Laser: f.eyeStyle = 4; f.eyeColor = ROSE; break;
        case Mood::Countdown: f.eyeStyle = 1; f.mouth = 3; break;
        case Mood::Bubbles: f.mouth = 2; f.eyeStyle = 2; break;
        case Mood::Rainbow: f.eyeStyle = 2; f.mouth = 1; break;
        case Mood::Peekaboo: f.winkLeft = elapsed % 500 < 200; f.winkRight = elapsed % 500 >= 200; f.mouth = 1; break;
        case Mood::Wave: f.winkRight = elapsed % 550 < 230; f.mouth = 1; break;
        case Mood::Squint: f.eyeHeight = 11; f.mouth = 3; break;
        case Mood::Glance: f.gazeX = elapsed % 600 < 300 ? -11 : 11; f.mouth = 1; break;
        case Mood::BigGrin: f.eyeStyle = 2; f.mouth = 4; f.cheeks = true; break;
        case Mood::Hiccup: f.eyeHeight = 17 + ix(fabsf(wave(now, .022f)) * 15); f.mouth = 2; break;
        case Mood::Sneeze: f.eyeStyle = elapsed < 600 ? 1 : 0; f.mouth = elapsed < 600 ? 2 : 4; break;
        case Mood::Victory: f.eyeStyle = 4; f.mouth = 4; f.eyeColor = GOLD; break;
        case Mood::Disco: f.eyeStyle = 2; f.mouth = 4; f.tilt = ix(wave(now, .018f) * 9); break;
        case Mood::Confetti: f.eyeStyle = 2; f.mouth = 4; break;
        case Mood::Orbit: f.gazeX = ix(11 * cosf(elapsed * .008f)); f.gazeY = ix(8 * sinf(elapsed * .008f)); break;
        case Mood::Fireworks: f.eyeStyle = 4; f.mouth = 2; f.eyeColor = GOLD; break;
        case Mood::PixelRain: f.eyeWidth = 24; f.eyeHeight = 24; f.mouth = 2; break;
        case Mood::CodeRain: f.gazeY = ix(wave(now, .005f) * 7); f.eyeColor = GREEN; break;
        case Mood::Rocket: f.eyeStyle = 4; f.mouth = 2; break;
        case Mood::Comet: f.gazeX = elapsed % 800 < 400 ? 10 : -10; f.mouth = 2; break;
        case Mood::CloudFloat: f.eyeHeight = 22; f.eyeStyle = 2; f.mouth = 1; break;
        case Mood::Rainy: f.eyeHeight = 24; f.mouth = 3; break;
        case Mood::Snowfall: f.eyeStyle = 2; f.mouth = 1; break;
        case Mood::Wiggle: f.tilt = ix(wave(now, .025f) * 8); f.mouth = 1; break;
        case Mood::Stretch: f.eyeHeight = 15 + ix(fabsf(wave(now, .006f)) * 26); f.mouth = 2; break;
        case Mood::Boop: f.eyeWidth = 39; f.eyeHeight = 40; f.mouth = 2; break;
        case Mood::Glitch: f.eyeColor = elapsed % 210 < 90 ? VIOLET : CYAN; f.gazeX = elapsed % 140 < 70 ? 7 : -7; f.mouth = 3; break;
        case Mood::Heartbeat: f.eyeStyle = 3; f.mouth = 1; f.eyeColor = ROSE; break;
        case Mood::Loading: f.eyeStyle = 1; f.eyebrow = 1; f.mouth = 3; break;
        case Mood::Deploy: f.eyeStyle = 4; f.mouth = 1; f.eyeColor = GREEN; break;
        case Mood::Build: f.eyeStyle = 2; f.mouth = 1; f.eyeColor = GREEN; break;
        case Mood::BugHunt: f.gazeX = elapsed % 450 < 225 ? -12 : 12; f.eyebrow = -1; break;
        case Mood::Coffee: f.eyeHeight = 15; f.mouth = 2; break;
        case Mood::Snack: f.mouth = 2; f.gazeY = 8; break;
        case Mood::Debug: f.eyeHeight = 23; f.eyebrow = 1; f.gazeX = -7; break;
        case Mood::HighFive: f.eyeStyle = 2; f.mouth = 4; break;
        case Mood::Panic: f.eyeWidth = 38; f.eyeHeight = 44; f.mouth = 2; f.eyeColor = ROSE; break;
        case Mood::Weekend: f.eyeStyle = 2; f.mouth = 4; f.tilt = ix(wave(now, .016f) * 8); break;
        case Mood::Rollback: f.eyeStyle = 5; f.mouth = 3; break;
        case Mood::SleepTalk: f.eyeStyle = 1; f.mouth = 1; break;
        case Mood::Ping: f.eyeWidth = 23; f.eyeHeight = 23; f.mouth = 2; break;
        case Mood::Typing: f.eyeHeight = 27; f.gazeY = 8; break;
        case Mood::Freeze: f.eyeWidth = 28; f.eyeHeight = 27; f.mouth = 0; break;
        case Mood::Zoomies: f.eyeStyle = 4; f.mouth = 4; f.tilt = ix(wave(now, .04f) * 13); break;
        case Mood::Noodle: f.mouth = 5; f.eyeHeight = 23; break;
        case Mood::Hypno: f.eyeStyle = 5; f.mouth = 1; break;
        case Mood::Magnet: f.eyeStyle = 3; f.mouth = 1; f.eyeColor = VIOLET; break;
        case Mood::PetBliss: f.eyeStyle = 2; f.mouth = 4; f.blush = true; break;
        case Mood::Purr: f.eyeStyle = 1; f.mouth = 1; f.blush = true; break;
        case Mood::Nuzzle: f.eyeStyle = 2; f.mouth = 1; f.tilt = ix(wave(now, .018f) * 6); break;
        case Mood::HeadTilt: f.eyebrow = 1; f.gazeX = 7; f.mouth = 1; f.tilt = 8; break;
        case Mood::HappyDance: f.eyeStyle = 2; f.mouth = 4; f.tilt = ix(wave(now, .021f) * 9); break;
        case Mood::Curious: f.eyebrow = 1; f.gazeX = 8; f.mouth = 2; break;
        case Mood::Cap: case Mood::Beanie: case Mood::TopHat:
        case Mood::WizardHat: case Mood::Crown: case Mood::PartyHat:
            f.eyeStyle = 2; f.mouth = 1; break;
        case Mood::Headphones: f.eyeStyle = 2; f.mouth = 4; f.tilt = ix(wave(now, .008f) * 5); break;
        case Mood::SleepCap: f.eyeStyle = 1; f.mouth = 1; break;
        case Mood::Sunglasses: f.mouth = 1; break;
        case Mood::Whistle: f.eyeStyle = 2; f.mouth = 2; break;
        case Mood::HelloWave: f.winkRight = elapsed % 750 < 300; f.mouth = 4; break;
        case Mood::PeekScreen: f.winkLeft = true; f.gazeX = -7; f.mouth = 1; break;
        case Mood::Bashful: f.eyeStyle = 2; f.mouth = 1; f.blush = true; break;
        case Mood::Alien: f.eyeColor = GREEN; f.eyeWidth = 38; f.eyeHeight = 42; f.mouth = 2; break;
        case Mood::SideShuffle: f.tilt = ix(wave(now, .022f) * 11); f.mouth = 1; break;
        case Mood::Wifi: f.eyeColor = SKY; f.eyeStyle = 2; f.mouth = 1; break;
        case Mood::Wake: f.eyeStyle = elapsed < 350 ? 1 : 0; f.mouth = 1; f.eyeHeight = elapsed < 300 ? 9 : 33; break;
        case Mood::Sleep: f.eyeStyle = 1; f.mouth = 0; break;
        case Mood::Dream: f.eyeStyle = 1; f.mouth = 1; break;
        case Mood::Neutral: default:
            f.gazeX = ix(wave(now, 0.00055f) * 3);
            f.gazeY = ix(wave(now, 0.0007f) * 2);
            // Quiet occasional blinking even between scheduled expressions.
            if (now % 4600 < 160) f.eyeStyle = 1;
            break;
    }
    return f;
}

template <typename G>
void drawEye(G &g, int x, int y, const Face &f, bool left) {
    if ((left && f.winkLeft) || (!left && f.winkRight) || f.eyeStyle == 1) {
        arc(g, x, y + 7, 16, 195, 345, f.eyeColor, 4);
        return;
    }
    if (f.eyeStyle == 2) {
        arc(g, x, y + 9, 20, 197, 343, f.eyeColor, 4);
        return;
    }
    if (f.eyeStyle == 3) { heart(g, x, y + 3, 1, ROSE); return; }
    if (f.eyeStyle == 4) { star(g, x, y + 1, 14, f.eyeColor); return; }
    if (f.eyeStyle == 5) {
        for (int i = 0; i < 27; ++i) {
            const float a = (i * 20.0f) * DEG;
            const float b = ((i + 1) * 20.0f) * DEG;
            const float r = 2.0f + 0.47f * i;
            g.drawLine(x + ix(cosf(a) * r), y + ix(sinf(a) * r),
                       x + ix(cosf(b) * (r + 0.47f)), y + ix(sinf(b) * (r + 0.47f)), f.eyeColor);
        }
        return;
    }
    const int px = x + f.gazeX;
    const int py = y + f.gazeY;
    g.fillRoundRect(px - f.eyeWidth / 2, py - f.eyeHeight / 2,
                    f.eyeWidth, f.eyeHeight, f.eyeWidth / 3, f.eyeColor);
    g.fillCircle(px - 5, py - 6, 3, WHITE);
    g.fillCircle(px + 8, py + 9, 2, SKY);
}

// Large, expressive mouths remain readable on the real 1.28-inch screen.
template <typename G>
void drawMouth(G &g, int x, int y, int type, uint16_t color) {
    if (type == 0) { arc(g, x, y - 8, 18, 22, 158, color, 4); return; }
    if (type == 1) { // warm, wide smile with corners
        arc(g, x, y - 13, 27, 17, 163, color, 5);
        g.fillCircle(x - 26, y - 5, 2, color);
        g.fillCircle(x + 26, y - 5, 2, color);
        return;
    }
    if (type == 2) {
        g.fillEllipse(x, y + 2, 14, 18, 0x0108);
        g.drawEllipse(x, y + 2, 15, 19, color);
        g.fillEllipse(x - 2, y + 10, 5, 3, ROSE);
        return;
    }
    if (type == 3) { arc(g, x, y + 14, 20, 195, 345, color, 4); return; }
    if (type == 4) { // open laugh / visible tongue
        g.fillEllipse(x, y + 1, 27, 15, 0x0108);
        arc(g, x, y - 11, 28, 8, 171, color, 5);
        g.fillRoundRect(x - 15, y + 8, 30, 6, 3, ROSE);
        return;
    }
    if (type == 5) {
        arc(g, x, y - 11, 25, 9, 167, color, 5);
        g.fillRoundRect(x - 11, y + 7, 22, 18, 8, ROSE);
        return;
    }
}

// Accessories and props are intentionally 40-60px, not tiny status icons.
// They overlap a portion of the face for a more playful character.
template <typename G>
void bigProp(G &g, Mood mood, uint32_t now) {
    if (mood == Mood::Coffee || mood == Mood::Snack) {
        const int y = 110 + ix(wave(now, .004f) * 3);
        g.fillRoundRect(158, y, 39, 36, 7, GOLD);
        g.fillRoundRect(162, y + 4, 31, 27, 5, 0x5A6B);
        g.drawCircle(201, y + 16, 11, GOLD);
        g.fillRect(155, y + 35, 48, 4, GOLD);
        for (int i = 0; i < 3; ++i)
            arc(g, 165 + i * 11, y - 12 + ix(wave(now, .004f, i) * 3), 9, 205, 340, SKY, 2);
    } else if (mood == Mood::BugHunt || mood == Mood::Debug || mood == Mood::Curious) {
        const int x = 170, y = 126 + ix(wave(now, .004f) * 4);
        g.fillCircle(x, y, 22, 0x144F);
        g.drawCircle(x, y, 22, GOLD);
        g.drawCircle(x, y, 19, SKY);
        g.drawLine(x + 15, y + 18, x + 36, y + 37, GOLD);
        g.drawLine(x + 17, y + 17, x + 38, y + 36, GOLD);
        spark(g, x - 8, y - 6, 5, WHITE);
    } else if (mood == Mood::Wifi || mood == Mood::Ping) {
        const int x = 167, y = 145;
        g.fillCircle(x, y - 15, 37, 0x0990);
        arc(g, x, y, 31, 215, 325, CYAN, 5);
        arc(g, x, y, 21, 215, 325, SKY, 4);
        arc(g, x, y, 11, 215, 325, WHITE, 3);
        g.fillCircle(x, y, 5, GOLD);
    } else if (mood == Mood::Idea) {
        const int x = 173, y = 116;
        g.fillCircle(x, y, 23, GOLD);
        g.fillRect(x - 12, y + 20, 24, 10, GOLD);
        g.fillRoundRect(x - 10, y + 29, 20, 7, 3, SKY);
        spark(g, x - 24, y - 23, 7, SKY);
    } else if (mood == Mood::Rocket || mood == Mood::Comet) {
        const int y = 103 - ix(4 * wave(now, .006f));
        g.fillTriangle(168, y - 13, 147, y + 33, 192, y + 28, WHITE);
        g.fillCircle(168, y + 12, 8, SKY);
        g.fillTriangle(147, y + 30, 140, y + 47, 158, y + 34, ROSE);
        g.fillTriangle(184, y + 29, 194, y + 45, 178, y + 34, GOLD);
        g.fillTriangle(161, y + 33, 177, y + 33, 169, y + 54, GOLD);
    } else if (mood == Mood::PetBliss || mood == Mood::Purr || mood == Mood::Nuzzle) {
        heart(g, 170 + ix(wave(now, .009f) * 7), 91 - ix((now % 900) / 130), 2, ROSE);
        arc(g, 107, 87, 24, 190, 336, GOLD, 3);
        arc(g, 120, 79, 24, 190, 336, SKY, 3);
        spark(g, 58, 140, 7, ROSE);
    }
}

template <typename G>
void headwear(G &g, Mood mood, int x, int y, uint32_t now) {
    // x is the moving head center, y is the crown of the head.
    if (mood == Mood::Cap || mood == Mood::PeekScreen) {
        g.fillRoundRect(x - 41, y - 10, 82, 23, 10, SKY);
        g.fillRoundRect(x - 6, y + 9, 57, 9, 4, CYAN);
        g.fillCircle(x - 18, y + 1, 4, GOLD);
    } else if (mood == Mood::Beanie || mood == Mood::Coffee || mood == Mood::Bashful) {
        g.fillRoundRect(x - 38, y - 17, 76, 31, 13, VIOLET);
        g.fillRoundRect(x - 39, y + 6, 78, 10, 4, SKY);
        g.fillCircle(x, y - 19, 9, ROSE);
    } else if (mood == Mood::TopHat) {
        g.fillRoundRect(x - 29, y - 30, 58, 47, 6, PANEL);
        g.fillRoundRect(x - 30, y + 8, 60, 8, 1, VIOLET);
        g.fillRoundRect(x - 49, y + 14, 98, 7, 3, CYAN);
    } else if (mood == Mood::WizardHat || mood == Mood::PartyHat) {
        g.fillTriangle(x, y - 52, x - 41, y + 19, x + 42, y + 19,
                       mood == Mood::WizardHat ? VIOLET : ROSE);
        g.fillRoundRect(x - 46, y + 13, 92, 9, 5, CYAN);
        star(g, x - 5, y - 11, 8, GOLD);
        if (mood == Mood::PartyHat) g.fillCircle(x, y - 54, 8, GOLD);
    } else if (mood == Mood::Crown) {
        g.fillTriangle(x - 42, y + 14, x - 39, y - 27, x - 16, y + 5, GOLD);
        g.fillTriangle(x - 18, y + 14, x, y - 38, x + 19, y + 14, GOLD);
        g.fillTriangle(x + 16, y + 5, x + 39, y - 27, x + 42, y + 14, GOLD);
        g.fillRoundRect(x - 42, y + 10, 84, 11, 3, GOLD);
        g.fillCircle(x, y + 15, 4, ROSE);
    } else if (mood == Mood::SleepCap || mood == Mood::SleepTalk) {
        g.fillTriangle(x - 39, y + 12, x + 4, y - 40, x + 44, y + 9, SKY);
        g.fillRoundRect(x - 40, y + 8, 83, 9, 4, WHITE);
        g.fillCircle(x + 4, y - 40, 8, GOLD);
    } else if (mood == Mood::Headphones || mood == Mood::Disco || mood == Mood::Weekend) {
        arc(g, x, y + 45, 65, 198, 342, CYAN, 5);
        g.fillRoundRect(x - 69, y + 34, 17, 48, 7, VIOLET);
        g.fillRoundRect(x + 52, y + 34, 17, 48, 7, VIOLET);
        g.fillRoundRect(x - 65, y + 39, 10, 36, 4, SKY);
        g.fillRoundRect(x + 55, y + 39, 10, 36, 4, SKY);
    } else if (mood == Mood::Sunglasses) {
        g.fillRoundRect(x - 55, y + 44, 48, 27, 9, 0x0108);
        g.fillRoundRect(x + 7, y + 44, 48, 27, 9, 0x0108);
        g.drawLine(x - 7, y + 52, x + 7, y + 52, GOLD);
        g.drawLine(x - 47, y + 50, x - 22, y + 50, SKY);
    }
    (void)now;
}

// A two-line speech bubble, not a small floating status label. Split at spaces
// without dynamic allocation, with at most 15 printable characters per line.
inline void speechLines(const char *caption, char *first, char *second) {
    constexpr int maxChars = 15;
    first[0] = second[0] = '\0';
    if (!caption) return;
    const size_t length = strlen(caption);
    if (length <= maxChars) {
        snprintf(first, 19, "%s", caption);
        return;
    }
    size_t cut = maxChars;
    while (cut > 7 && caption[cut] != ' ') --cut;
    if (cut <= 7) cut = maxChars;
    size_t part = cut;
    if (part > 18) part = 18;
    memcpy(first, caption, part);
    first[part] = '\0';
    const char *rest = caption + cut;
    while (*rest == ' ') ++rest;
    snprintf(second, 19, "%s", rest);
}

template <typename G>
void speechBubble(G &g, const char *caption, bool positive) {
    if (!caption || !*caption) return;
    char line1[19] = {}, line2[19] = {};
    speechLines(caption, line1, line2);
    // Speech tail points towards Buddy's mouth.
    g.fillTriangle(143, 175, 159, 161, 163, 178, PANEL);
    g.fillRoundRect(33, 171, 174, 42, 15, PANEL);
    g.drawRoundRect(33, 171, 174, 42, 15, positive ? SKY : GOLD);
    if (line2[0]) {
        text(g, line1, 120, 182, 2, WHITE);
        text(g, line2, 120, 201, 2, positive ? CYAN : GOLD);
    } else {
        text(g, line1, 120, 192, 2, positive ? CYAN : WHITE);
    }
}

template <typename G>
void buddyExtras(G &g, Mood mood, uint32_t elapsed, uint32_t now) {
    const float t = elapsed * 0.001f;
    switch (mood) {
        case Mood::Hearts:
            heart(g, 37 + ix(wave(now, .005f) * 7), 62 - ix(t * 8), 1, ROSE);
            heart(g, 204, 57 - ix(t * 9), 1, ROSE);
            break;
        case Mood::Sparkles:
        case Mood::StarEyes:
            for (int i = 0; i < 7; ++i) {
                const float a = i * 0.9f + t;
                spark(g, 120 + ix(cosf(a) * (66 + 9 * sinf(a))),
                      121 + ix(sinf(a) * (67 + 8 * cosf(a))), (i % 3) + 2,
                      i % 2 ? GOLD : CYAN);
            }
            break;
        case Mood::Blush:
        case Mood::Shy:
            for (int i = 0; i < 3; ++i) {
                g.drawLine(39 + i * 6, 159, 44 + i * 6, 151, ROSE);
                g.drawLine(185 + i * 6, 151, 190 + i * 6, 159, ROSE);
            }
            
            break;
        case Mood::Party:
        case Mood::Bounce:
        case Mood::Giggle:
            for (int i = 0; i < 7; ++i) {
                const float a = i * .9f + t * 1.5f;
                spark(g, 120 + ix(cosf(a) * 89), 118 + ix(sinf(a) * 88),
                      3 + i % 3, i % 2 ? VIOLET : GOLD);
            }
            break;
        case Mood::Thinking:
        case Mood::Idea:
            g.drawCircle(192, 78, 7, SKY);
            g.drawCircle(201, 64, 11, SKY);
            if (mood == Mood::Idea) {
                spark(g, 201, 62, 8, GOLD);

            } else {

            }
            break;
        case Mood::Sing:
            text(g, "~", 205, 69 + ix(sinf(t * 6) * 9), 4, GOLD);
            text(g, "*", 31, 91 + ix(sinf(t * 7) * 6), 4, CYAN);
            break;
        case Mood::Tears:
            g.fillCircle(73, 159 + ix(t * 9) % 13, 5, SKY);
            g.fillCircle(167, 159 + ix(t * 10) % 13, 5, SKY);
            break;
        case Mood::Laser:
            g.drawLine(79, 123, 43 + ix(12 * sinf(t * 4)), 38, ROSE);
            g.drawLine(161, 123, 199 - ix(12 * sinf(t * 4)), 38, ROSE);
            break;
        case Mood::Dizzy:
            for (int i = 0; i < 4; ++i) {
                const float a = t * 5 + (i * PI_F * .5f);
                star(g, 120 + ix(cosf(a) * 73), 121 + ix(sinf(a) * 73), 5, GOLD);
            }
            break;
        case Mood::Countdown: {
            const int n = 3 - static_cast<int>(elapsed / 500) % 3;
            char value[4]; snprintf(value, sizeof(value), "%d", n);
            text(g, value, 120, 52, 4, GOLD);
            break;
        }
        case Mood::Bubbles:
            for (int i = 0; i < 8; ++i) {
                const int x = 44 + (i * 37) % 151;
                const int y = 182 - (ix(t * (13 + i * 3)) % 115);
                g.drawCircle(x, y, 3 + i % 4, i % 2 ? SKY : VIOLET);
            }
            break;
        case Mood::Rainbow:
            arc(g, 120, 120, 89, 200, 340, ROSE, 3);
            arc(g, 120, 120, 94, 200, 340, GOLD, 3);
            arc(g, 120, 120, 99, 200, 340, SKY, 3);
            break;
        case Mood::Wave:
        case Mood::HighFive:
            for (int i = 0; i < 3; ++i) {
                arc(g, 205, 121, 9 + i * 8, 220 + ix(12 * wave(now, .009f)), 320, i == 0 ? CYAN : SKY, 2);
            }
            break;
        case Mood::Disco:
        case Mood::Zoomies:
        case Mood::Wiggle:
        case Mood::Weekend:
            for (int i = 0; i < 9; ++i) {
                const float a = t * 5 + i * .72f;
                spark(g, 120 + ix(cosf(a) * 92), 121 + ix(sinf(a) * 92), 2 + i % 4,
                      i % 2 ? ROSE : GOLD);
            }
            break;
        case Mood::Victory:
        case Mood::Confetti:
        case Mood::Fireworks:
            for (int i = 0; i < 14; ++i) {
                const float a = i * .49f + t * (i % 3 == 0 ? -2 : 2);
                const int r = 41 + ix(fmodf(t * (28 + i * 3), 61));
                spark(g, 120 + ix(cosf(a) * r), 120 + ix(sinf(a) * r), 2 + i % 3,
                      i % 3 == 0 ? CYAN : (i % 3 == 1 ? ROSE : GOLD));
            }
            break;
        case Mood::Orbit:
        case Mood::Magnet:
            for (int i = 0; i < 3; ++i) {
                const float a = t * 3 + i * 2.094f;
                g.fillCircle(120 + ix(cosf(a) * 95), 120 + ix(sinf(a) * 95), 3 + i, i == 0 ? ROSE : CYAN);
            }
            break;
        case Mood::PixelRain:
        case Mood::CodeRain:
            for (int i = 0; i < 14; ++i) {
                const int x = 32 + (i * 41) % 174;
                const int y = 35 + (ix(t * (18 + i * 3)) + i * 19) % 168;
                g.fillRect(x, y, 2 + i % 3, 3 + i % 5, mood == Mood::CodeRain ? GREEN : SKY);
            }
            break;
        case Mood::Rocket:
        case Mood::Comet:
            for (int i = 0; i < 6; ++i) {
                const int x = 36 + (ix(t * 55) + i * 23) % 170;
                const int y = 196 - (i * 29 + ix(t * 25)) % 140;
                g.drawLine(x, y, x - 6 - i % 3, y + 6 + i % 4, i % 2 ? GOLD : CYAN);
            }
            break;
        case Mood::CloudFloat:
            for (int i = 0; i < 3; ++i) {
                const int x = 33 + i * 74 + ix(wave(now, .002f, i) * 8);
                g.drawCircle(x, 61 + i % 2 * 18, 7 + i % 2 * 3, SKY);
            }
            break;
        case Mood::Rainy:
        case Mood::Snowfall:
            for (int i = 0; i < 13; ++i) {
                const int x = 42 + (i * 37) % 155;
                const int y = 38 + (ix(t * (24 + i * 3)) + i * 31) % 161;
                if (mood == Mood::Rainy) g.drawLine(x, y, x - 3, y + 6, SKY);
                else spark(g, x, y, 2, WHITE);
            }
            break;
        case Mood::Hiccup:
        case Mood::Sneeze:
        case Mood::Boop:
            for (int i = 0; i < 4; ++i) {
                const float a = t * 7 + i * 1.57f;
                spark(g, 120 + ix(cosf(a) * 74), 123 + ix(sinf(a) * 70), 2 + i, GOLD);
            }
            break;
        case Mood::Stretch:
            arc(g, 120, 120, 94, 220, 310 + 45 * wave(now, .005f), SKY, 2);
            break;
        case Mood::Glitch:
            for (int i = 0; i < 4; ++i) {
                const int y = 67 + i * 33 + ix(wave(now, .023f, i) * 4);
                g.drawLine(32, y, 48 + i * 12, y, i % 2 ? VIOLET : ROSE);
            }
            break;
        case Mood::Heartbeat:
            heart(g, 204, 69 + ix(wave(now, .01f) * 4), 1, ROSE);
            break;
        case Mood::Loading:
        case Mood::Build:
        case Mood::Deploy:
            for (int i = 0; i < 7; ++i) {
                const float a = (i * 51 + ix(t * 135)) * DEG;
                g.fillCircle(120 + ix(cosf(a) * 92), 120 + ix(sinf(a) * 92), 2 + i % 2, i % 2 ? GREEN : CYAN);
            }
            break;
        case Mood::BugHunt:
        case Mood::Debug:
        case Mood::Coffee:
        case Mood::Snack:
            break; // Bigger foreground props are drawn in bigProp().
        case Mood::Panic:
        case Mood::Rollback:
            for (int i = 0; i < 4; ++i)
                spark(g, 36 + i * 56, 55 + (i % 2) * 141, 5, ROSE);
            break;
        case Mood::Ping:
            for (int i = 0; i < 3; ++i)
                g.drawCircle(120, 120, 64 + i * 13 + ix(fabsf(wave(now, .007f)) * 3), i % 2 ? SKY : DIM);
            break;
        case Mood::Typing:
            for (int i = 0; i < 7; ++i)
                g.fillRoundRect(49 + i * 23, 199, 16, 7, 2, (elapsed / 125 + i) % 3 == 0 ? CYAN : DIM);
            break;
        case Mood::Freeze:
            for (int i = 0; i < 7; ++i)
                spark(g, 48 + i * 24, 188 - i % 3 * 13, 3, SKY);
            break;
        case Mood::Noodle:
            for (int i = 0; i < 4; ++i)
                arc(g, 120, 167, 15 + i * 5, 6 + 12 * sinf(t * 5), 135 + i * 8, ROSE, 2);
            break;
        case Mood::Hypno:
            for (int i = 0; i < 3; ++i)
                arc(g, 120, 122, 58 + i * 14, 10 + ix(t * 70), 160 + ix(t * 70), VIOLET, 2);
            break;
        case Mood::Sleep:
        case Mood::Sleepy:
        case Mood::Dream:
        case Mood::SleepTalk:
            text(g, "z", 202, 73 - ix((now % 3100) / 90), 4, SKY);
            text(g, "Z", 214, 52 - ix((now % 2600) / 90), 4, VIOLET);
            if (mood == Mood::Dream) star(g, 38, 47, 8, GOLD);
            break;
        case Mood::Confused: text(g, "?", 197, 55, 4, GOLD); break;
        case Mood::Surprise: text(g, "!", 202, 57, 4, GOLD); break;
        case Mood::Grumpy: spark(g, 34, 89, 5, ROSE); break;
        case Mood::Mischief:
        case Mood::Peekaboo:
        case Mood::Yawn:
        case Mood::Wake: break;
        default: break;
    }
}

template <typename G>
void buddy(G &g, uint32_t now, Mood mood, uint32_t elapsed, const char *quip,
           bool volumeVisible, int volumePct, bool usbConnected) {
    base(g, now);
    // Restrained premium UI with a small, animated character in a round glass.
    arc(g, 120, 120, 105, 205, 324, VIOLET, 2);
    arc(g, 120, 120, 103, 20, 146, SKY, 2);
    const bool sleeping = mood == Mood::Sleep || mood == Mood::Dream ||
                          mood == Mood::SleepTalk || mood == Mood::SleepCap;
    const bool petting = mood == Mood::PetBliss || mood == Mood::Purr ||
                         mood == Mood::Nuzzle;
    const int bobX = ix(wave(now, .0015f, .4f) * 7) + ix(wave(now, .003f) * 2);
    const int bobY = ix(wave(now, sleeping ? .0016f : .0024f) * 4);
    const int cx = 120 + bobX;
    const int cy = 120 + bobY;
    g.fillCircle(cx, cy, 79, EDGE);
    g.fillCircle(cx, cy, 75, INNER);
    g.drawCircle(cx, cy, 75, sleeping ? DIM : PANEL);

    Face f = faceFor(mood, elapsed, now);
    if (sleeping) f.eyeColor = SKY;
    const int eyeL = cx - 30;
    const int eyeR = cx + 30;
    drawEye(g, eyeL, cy - 5 + f.tilt, f, true);
    drawEye(g, eyeR, cy - 5 - f.tilt, f, false);
    drawMouth(g, cx, cy + 32, f.mouth, f.eyeColor);
    if (f.eyebrow != 0) {
        const int sign = f.eyebrow;
        g.drawLine(cx - 49, cy - 40 - sign * 5, cx - 20, cy - 37 + sign * 5, f.eyeColor);
        g.drawLine(cx + 20, cy - 37 + sign * 5, cx + 49, cy - 40 - sign * 5, f.eyeColor);
    }
    if (f.blush || f.cheeks || petting) {
        g.fillEllipse(cx - 54, cy + 23, 13, 6, ROSE);
        g.fillEllipse(cx + 54, cy + 23, 13, 6, ROSE);
    }

    headwear(g, mood, cx, cy - 64, now);
    buddyExtras(g, mood, elapsed, now);
    bigProp(g, mood, now);

    // A pet response is intentionally cheerful and visibly distinct.
    if (petting) {
        for (int i = 0; i < 3; ++i) {
            const int drift = ix((elapsed * (i + 2) / 50) % 18);
            spark(g, 59 + i * 19, 101 - drift, 3 + i, i % 2 ? GOLD : CYAN);
        }
    }

    if (volumeVisible) {
        drawVolumeRing(g, volumePct, now, true);
        char label[16]; snprintf(label, sizeof(label), "%d%%", volumePct);
        g.fillRoundRect(79, 181, 82, 31, 14, PANEL);
        text(g, label, 120, 196, 2, WHITE);
    } else if (quip && *quip) {
        speechBubble(g, quip, petting || mood == Mood::HelloWave || mood == Mood::Happy);
    } else {
        text(g, sleeping ? "SLEEP MODE" : "SWIPE  /  PET", 120, 207, 1, MUTED);
    }
    g.fillCircle(202, 194, 3, usbConnected ? GREEN : ROSE);
    pageDots(g, 1);
}

template <typename G>
void transition(G &g, uint32_t elapsed, uint32_t total, bool toBuddy) {
    if (elapsed >= total) return;
    const float t = static_cast<float>(elapsed) / total;
    const float wavePhase = t * 2 * PI_F;
    const int radius = 24 + ix(103 * sinf(wavePhase * .5f));
    arc(g, 120, 121, radius, 220, 420, toBuddy ? CYAN : VIOLET, 3);
    spark(g, 120 + ix(cosf(wavePhase) * radius),
          121 + ix(sinf(wavePhase) * radius), 5, toBuddy ? GOLD : SKY);
}
}
