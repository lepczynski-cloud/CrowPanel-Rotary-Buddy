#include <cassert>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include "BuddyBrain.h"
#include "MediaLogic.h"
#include "TouchGesture.h"
#include "RotaryDecoder.h"
#include "VolumeScaler.h"

int main() {
    static_assert(BUDDY_MOOD_COUNT == 107, "Unexpected number of animation states");
    static_assert(BUDDY_QUIP_COUNT == 108, "Unexpected number of joke captions");
    assert(std::string(moodName(Mood::Neutral)) == "Neutral");
    assert(std::string(moodName(Mood::Dream)) == "Dream");
    assert(std::string(moodName(Mood::Count)) == "Unknown");

    // Media targets must not overlap, including the central Play/Pause target.
    assert(mediaHitTest(120, 119) == MediaButton::PlayPause);
    assert(mediaHitTest(47, 119) == MediaButton::Previous);
    assert(mediaHitTest(193, 119) == MediaButton::Next);
    assert(mediaHitTest(120, 188) == MediaButton::Mute);
    assert(mediaHitTest(120, 20) == MediaButton::None);
    assert(mediaHitTest(5, 5) == MediaButton::None);
    for (int x = 0; x < 240; ++x) {
        for (int y = 0; y < 240; ++y) {
            const MediaButton zone = mediaHitTest(x, y);
            assert(static_cast<int>(zone) >= -1 && static_cast<int>(zone) <= 3);
        }
    }

    // Swipe is recognized while dragging, and cannot generate a tap on release.
    TouchGesture gesture;
    auto consume = [&](bool press, bool release, int x, int y, unsigned long now) {
        return gesture.consume(press, release, 0, x, y, now, 12, 500, 19, 43);
    };
    assert(consume(true, false, 191, 121, 1000).type == GestureType::None);
    assert(consume(true, false, 130, 122, 1060).type == GestureType::SwipeLeft);
    assert(consume(true, false, 63, 123, 1080).type == GestureType::None);
    assert(consume(false, true, 63, 123, 1140).type == GestureType::None);
    // Downward stroking is distinct from navigation, and never sends a tap.
    assert(consume(true, false, 119, 76, 1500).type == GestureType::None);
    assert(consume(true, false, 116, 149, 1570).type == GestureType::PetDown);
    assert(consume(false, true, 116, 149, 1600).type == GestureType::None);
    // Upward movement has no host action and never navigates.
    assert(consume(true, false, 120, 166, 1700).type == GestureType::None);
    assert(consume(true, false, 120, 96, 1770).type == GestureType::SwipeUp);
    assert(consume(false, true, 120, 96, 1810).type == GestureType::None);
    // A vertical-looking hardware ID cannot turn a horizontal drag into petting.
    assert(TouchGesture::hardwareDirection(1) == GestureType::PetDown);
    assert(TouchGesture::hardwareDirection(3) == GestureType::SwipeLeft);
    TouchGesture safety;
    safety.consume(true, false, 1, 120, 120, 6000, 12, 500, 19, 43);
    assert(safety.consume(true, false, 1, 160, 123, 6030, 12, 500, 19, 43).type == GestureType::None);
    assert(safety.consume(true, false, 3, 66, 123, 6060, 12, 500, 19, 43).type == GestureType::SwipeLeft);

    assert(consume(true, false, 120, 121, 2000).type == GestureType::None);
    assert(consume(false, true, 121, 122, 2100).type == GestureType::Tap);
    assert(consume(true, false, 120, 121, 2400).type == GestureType::None);
    assert(consume(true, false, 141, 148, 2440).type == GestureType::None);
    assert(consume(false, true, 141, 148, 2510).type == GestureType::None);

    // Volume scaling remains half speed by default.
    VolumeScaler scaler(2);
    assert(scaler.apply(3) == 1);
    assert(scaler.apply(3) == 2);
    assert(scaler.apply(-5) == -2); // direction change clears remainder
    assert(scaler.apply(-5) == -3);
    scaler.reset();
    assert(scaler.apply(1) == 0);

    RotaryDecoder rotary(4);
    rotary.reset(false, false);
    int clockwise = 0;
    clockwise += rotary.update(true, false);
    clockwise += rotary.update(true, true);
    clockwise += rotary.update(false, true);
    clockwise += rotary.update(false, false);
    assert(clockwise == 1);
    int anticlockwise = 0;
    anticlockwise += rotary.update(false, true);
    anticlockwise += rotary.update(true, true);
    anticlockwise += rotary.update(true, false);
    anticlockwise += rotary.update(false, false);
    assert(anticlockwise == -1);

    // Local dial volume is explicitly an ESTIMATE. Clamp & animate gradually.
    VolumeIndicator volume(50, 2);
    assert(volume.displayed() == 50);
    volume.report(7, 1000);
    assert(volume.target() == 64 && volume.visible(2000, 2300));
    volume.update();
    assert(volume.displayed() > 50 && volume.displayed() < 64);
    for (int i = 0; i < 40; ++i) volume.update();
    assert(volume.displayed() == 64);
    volume.report(-999, 5000);
    assert(volume.target() == 0);
    volume.report(1000, 5200);
    assert(volume.target() == 100);
    volume.dismiss();
    assert(!volume.visible(5300, 2300));

    // Autopage timing and uint32 wraparound.
    assert(!shouldAutoBuddy(29000, 0, true, 30000));
    assert(shouldAutoBuddy(30000, 0, true, 30000));
    assert(!shouldAutoBuddy(90000, 0, false, 30000));
    const uint32_t rollover = 0xfffffff0u;
    assert(shouldAutoBuddy(14, rollover, true, 30));

    BuddyBrain brain(0x1234567u);
    brain.begin(1000);
    assert(brain.mood() == Mood::Wake);
    brain.update(2500, 300000);
    assert(brain.mood() == Mood::Neutral);
    brain.activity(2600, true);
    assert(brain.mood() != Mood::Neutral);
    brain.update(2600 + 300001, 300000);
    assert(brain.sleeping() && brain.mood() == Mood::Sleepy);
    brain.update(2600 + 302500, 300000);
    assert(brain.mood() == Mood::Sleep);
    brain.activity(2600 + 302600, true);
    assert(!brain.sleeping() && brain.mood() == Mood::Wake);
    brain.pet(2600 + 305000);
    assert(!brain.sleeping());
    assert(brain.quip() && std::strlen(brain.quip()) > 0);
    assert(brain.mood() == Mood::PetBliss || brain.mood() == Mood::Purr ||
           brain.mood() == Mood::Nuzzle || brain.mood() == Mood::Happy ||
           brain.mood() == Mood::Blush || brain.mood() == Mood::Bashful);


    // Deterministic random scheduler: all 107 moods defined, many reachable idle
    // reactions actually become reachable, no tight endless animation loop.
    BuddyBrain variety(0x45221923u);
    variety.begin(0);
    std::set<unsigned> ids;
    for (uint32_t now = 0; now < 20000000u; now += 133) {
        variety.update(now, 2100000000u);
        ids.insert(static_cast<unsigned>(variety.mood()));
    }
    assert(ids.size() >= 65);
    // Boop reactions can display jokes and never call USB host actions.
    BuddyBrain playful(0x5199u);
    playful.begin(0);
    unsigned captions = 0;
    for (uint32_t i = 1; i < 2000; ++i) {
        playful.activity(i * 100, true);
        if (playful.quip()) ++captions;
    }
    assert(captions > 700);
    // A sleeping buddy sometimes sleep-talks, and touch wakes it reliably.
    BuddyBrain sleeper(0x913a);
    sleeper.begin(0);
    sleeper.update(301000, 300000);
    assert(sleeper.sleeping());
    bool heardSleepTalk = false;
    for (uint32_t now = 304000; now < 1800000; now += 5000) {
        sleeper.update(now, 300000);
        if (sleeper.mood() == Mood::SleepTalk) heardSleepTalk = true;
    }
    assert(heardSleepTalk);
    sleeper.activity(1800500, true);
    assert(!sleeper.sleeping());
    std::cout << "PASS: gesture safety, encoder, media zones, volume estimate, sleep/wake, "
              << ids.size() << " sampled animation states\n";
}
