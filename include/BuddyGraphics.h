#pragma once
#include <Arduino.h>
#include <math.h>
#include <stdio.h>
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
void mediaGlyph(G &g, int id, int cx, int cy, uint16_t color) {
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
    } else { // mute: loudspeaker with slash
        g.fillRect(cx - 15, cy - 5, 9, 10, color);
        g.fillTriangle(cx - 9, cy - 5, cx + 2, cy - 13, cx + 2, cy + 13, color);
        g.drawLine(cx + 7, cy - 10, cx + 18, cy + 10, ROSE);
        g.drawLine(cx + 9, cy - 10, cx + 20, cy + 10, ROSE);
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
           int feedbackKind, uint32_t feedbackElapsed, bool usbConnected) {
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
            mediaGlyph(g, i, locations[i][0], locations[i][1], i == 1 ? BG : WHITE);
        }
        text(g, "PREV", 47, 83, 1, MUTED);
        text(g, "NEXT", 193, 83, 1, MUTED);
        text(g, "MUTE", 120, 208, 1, MUTED);
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
    int eyeWidth = 32;
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

template <typename G>
void drawMouth(G &g, int x, int y, int type, uint16_t color) {
    if (type == 0) { arc(g, x, y, 6, 22, 158, color, 3); return; }
    if (type == 1) { arc(g, x, y - 6, 18, 15, 165, color, 4); return; }
    if (type == 2) { g.fillEllipse(x, y + 2, 10, 15, INNER); g.drawEllipse(x, y + 2, 11, 16, color); return; }
    if (type == 3) { arc(g, x, y + 18, 17, 197, 343, color, 3); return; }
    if (type == 4) {
        g.fillEllipse(x, y + 2, 21, 12, INNER);
        arc(g, x, y - 6, 22, 5, 174, color, 4);
        g.fillRoundRect(x - 12, y + 7, 24, 4, 2, color);
        return;
    }
    if (type == 5) {
        arc(g, x, y - 6, 19, 6, 170, color, 3);
        g.fillRoundRect(x - 9, y + 7, 18, 13, 6, ROSE);
        return;
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
            if (mood == Mood::Shy) text(g, "HI...", 120, 48, 2, SKY);
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
                text(g, "NICE!", 120, 48, 2, GOLD);
            } else {
                text(g, "COFFEE?", 120, 48, 2, SKY);
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
        case Mood::Sleep:
        case Mood::Sleepy:
        case Mood::Dream:
            text(g, "z", 202, 73 - ix((now % 3100) / 90), 4, SKY);
            text(g, "Z", 214, 52 - ix((now % 2600) / 90), 4, VIOLET);
            if (mood == Mood::Dream) star(g, 38, 47, 8, GOLD);
            break;
        case Mood::Confused: text(g, "?", 197, 55, 4, GOLD); break;
        case Mood::Surprise: text(g, "!", 202, 57, 4, GOLD); break;
        case Mood::Grumpy: spark(g, 34, 89, 5, ROSE); break;
        case Mood::Mischief: text(g, "PSST!", 120, 48, 2, VIOLET); break;
        case Mood::Peekaboo: text(g, "BOO!", 120, 48, 2, GOLD); break;
        case Mood::Yawn: text(g, "YAWN", 120, 49, 2, SKY); break;
        case Mood::Wake: text(g, "HELLO!", 120, 47, 2, CYAN); break;
        default: break;
    }
}

template <typename G>
void buddy(G &g, uint32_t now, Mood mood, uint32_t elapsed,
           bool volumeVisible, int volumePct, bool usbConnected) {
    base(g, now);
    // Ambient aurora loops and engraved faceplate create depth without images.
    arc(g, 120, 120, 106, 210, 325, VIOLET, 2);
    arc(g, 120, 120, 100, 18, 143, SKY, 2);
    const bool sleeping = mood == Mood::Sleep || mood == Mood::Dream;
    const float breathe = sleeping ? 1.5f * wave(now, .0018f) : 1.0f * wave(now, .002f);
    g.fillCircle(120, 123, 90 + ix(breathe), EDGE);
    g.fillCircle(120, 123, 83 + ix(breathe), INNER);
    g.drawCircle(120, 123, 83 + ix(breathe), sleeping ? DIM : PANEL);
    if (volumeVisible) {
        drawVolumeRing(g, volumePct, now, true);
        char label[16]; snprintf(label, sizeof(label), "%d%%", volumePct);
        g.fillRoundRect(78, 187, 84, 24, 12, PANEL);
        text(g, label, 120, 199, 2, WHITE);
        text(g, "EST.", 120, 215, 1, MUTED);
    }
    Face f = faceFor(mood, elapsed, now);
    if (sleeping) f.eyeColor = SKY;
    const int tilt = f.tilt;
    // A subtle floating head motion without expensive bitmap scaling.
    const int bob = ix(wave(now, .0015f, .9f) * 2.0f);
    drawEye(g, 79, 119 + tilt + bob, f, true);
    drawEye(g, 161, 119 - tilt + bob, f, false);
    drawMouth(g, 120, 163 + bob, f.mouth, f.eyeColor);
    if (f.eyebrow != 0) {
        const int sign = f.eyebrow;
        g.drawLine(60, 83 - sign * 5, 95, 86 + sign * 5, f.eyeColor);
        g.drawLine(145, 86 + sign * 5, 180, 83 - sign * 5, f.eyeColor);
    }
    buddyExtras(g, mood, elapsed, now);
    if (!volumeVisible) {
        text(g, sleeping ? "DREAM MODE" : "TAP TO PLAY", 120, 210, 1, MUTED);
    }
    g.fillCircle(200, 196, 3, usbConnected ? GREEN : ROSE);
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
