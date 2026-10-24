#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "BuddyGraphics.h"

// Compile and exercise the ENTIRE templated renderer without hardware. Catch
// unsupported drawing API calls or broken switch/caption implementations.
struct MockScreen {
    unsigned operations = 0;
    template <typename... Args> void fillScreen(Args...) { ++operations; }
    template <typename... Args> void drawCircle(Args...) { ++operations; }
    template <typename... Args> void fillCircle(Args...) { ++operations; }
    template <typename... Args> void drawEllipse(Args...) { ++operations; }
    template <typename... Args> void fillEllipse(Args...) { ++operations; }
    template <typename... Args> void fillRect(Args...) { ++operations; }
    template <typename... Args> void drawLine(Args...) { ++operations; }
    template <typename... Args> void drawRoundRect(Args...) { ++operations; }
    template <typename... Args> void fillRoundRect(Args...) { ++operations; }
    template <typename... Args> void fillTriangle(Args...) { ++operations; }
    template <typename... Args> void drawString(Args...) { ++operations; }
    template <typename... Args> void setTextColor(Args...) { ++operations; }
    template <typename... Args> void setTextDatum(Args...) { ++operations; }
    template <typename... Args> void setTextFont(Args...) { ++operations; }
    template <typename... Args> void setTextSize(Args...) { ++operations; }
};
int main() {
    static_assert(BUDDY_MOOD_COUNT == 85, "Track the public animation count");
    static_assert(BUDDY_QUIP_COUNT == 72, "Track the public quip count");
    MockScreen g;
    for (unsigned i = 0; i < BUDDY_MOOD_COUNT; ++i) {
        const auto m = static_cast<Mood>(i);
        const auto f = Art::faceFor(m, 1100, 22000);
        assert(f.eyeWidth > 0 && f.eyeHeight > 0);
        Art::buddy(g, 20000, m, 700, nullptr, false, 50, true);
        Art::buddy(g, 21000, m, 1550, "LET'S GO PROD!", false, 50, true);
        Art::buddy(g, 22000, m, 2000, "HELLO, WORLD!", true, 80, true);
    }
    for (unsigned i = 0; i < BUDDY_QUIP_COUNT; ++i) {
        assert(std::strlen(BUDDY_QUIPS[i].line) <= 24);
        assert(std::strlen(BUDDY_QUIPS[i].line) > 0);
        assert(static_cast<unsigned>(BUDDY_QUIPS[i].mood) < BUDDY_MOOD_COUNT);
    }
    Art::media(g, 25000, 60, true, 1, 240, true);
    Art::media(g, 26000, 60, false, 3, 500, true);
    Art::transition(g, 200, 340, true);
    assert(g.operations > 2000);
    std::cout << "PASS: " << BUDDY_MOOD_COUNT << " rendered moods, "
              << BUDDY_QUIP_COUNT << " captions, media controls and transitions ("
              << g.operations << " drawing operations)\n";
}
