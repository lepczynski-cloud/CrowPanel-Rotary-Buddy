#pragma once
#include <stdint.h>

// Hardware-independent animation scheduler. No network, AI, microphone, or
// copyrighted character assets. The face and effects are rendered procedurally.
enum class Mood : uint8_t {
    Neutral,
    Blink, DoubleBlink, LookLeft, LookRight, LookUp, LookDown,
    WinkLeft, WinkRight, Happy, Laugh, Blush, Hearts, StarEyes,
    Sparkles, Surprise, Confused, Thinking, SideEye, Pout,
    Grumpy, Mischief, Tongue, Giggle, Party, Bounce, Dizzy,
    Peek, Shy, Yawn, Sing, Tears, Sleepy, RollEyes, Idea,
    Laser, Countdown, Bubbles, Rainbow, Peekaboo, Wake, Sleep, Dream,
    Count
};

constexpr unsigned BUDDY_MOOD_COUNT = static_cast<unsigned>(Mood::Count);

inline const char *moodName(Mood mood) {
    static constexpr const char *names[] = {
        "Neutral", "Blink", "Double blink", "Look left", "Look right",
        "Look up", "Look down", "Wink left", "Wink right", "Happy",
        "Laugh", "Blush", "Hearts", "Star eyes", "Sparkles", "Surprise",
        "Confused", "Thinking", "Side eye", "Pout", "Grumpy",
        "Mischief", "Tongue", "Giggle", "Party", "Bounce", "Dizzy",
        "Peek", "Shy", "Yawn", "Sing", "Tears", "Sleepy",
        "Roll eyes", "Idea", "Laser", "Countdown", "Bubbles",
        "Rainbow", "Peekaboo", "Wake", "Sleep", "Dream"
    };
    const unsigned index = static_cast<unsigned>(mood);
    return index < BUDDY_MOOD_COUNT ? names[index] : "Unknown";
}

class BuddyBrain {
public:
    explicit BuddyBrain(uint32_t seed = 0x91A73BD2u) : rng_(seed ? seed : 1u) {}

    void begin(uint32_t now) {
        lastInteraction_ = now;
        startedAt_ = now;
        current_ = Mood::Wake;
        activeUntil_ = now + 1400;
        nextIdleAt_ = now + 8000;
        sleeping_ = false;
    }

    void activity(uint32_t now, bool playful = false) {
        const bool wasSleeping = sleeping_;
        sleeping_ = false;
        lastInteraction_ = now;
        if (wasSleeping) {
            play(Mood::Wake, now, 1600);
        } else if (playful) {
            static constexpr Mood reactions[] = {
                Mood::Happy, Mood::Laugh, Mood::Blush, Mood::Hearts,
                Mood::StarEyes, Mood::Surprise, Mood::Pout, Mood::Tongue,
                Mood::Giggle, Mood::Party, Mood::Dizzy, Mood::Shy,
                Mood::Laser, Mood::Peekaboo, Mood::Idea, Mood::Bubbles,
                Mood::Rainbow, Mood::Mischief
            };
            constexpr uint32_t count = sizeof(reactions) / sizeof(reactions[0]);
            Mood next = reactions[random(count)];
            if (next == current_) next = reactions[(random(count - 1) + 1) % count];
            play(next, now, 1600 + random(950));
        }
    }

    void update(uint32_t now, uint32_t sleepAfterMs) {
        if (!sleeping_ && (now - lastInteraction_) >= sleepAfterMs) {
            sleeping_ = true;
            play(Mood::Sleepy, now, 2100);
            return;
        }
        if (current_ != Mood::Neutral && current_ != Mood::Sleep &&
            static_cast<int32_t>(now - activeUntil_) >= 0) {
            current_ = sleeping_ ? Mood::Sleep : Mood::Neutral;
            startedAt_ = now;
            nextIdleAt_ = now + (sleeping_ ? 16000 + random(12000) : 4800 + random(8400));
        }
        if (static_cast<int32_t>(now - nextIdleAt_) < 0) return;
        if (sleeping_) {
            play(Mood::Dream, now, 1800 + random(1100));
            nextIdleAt_ = now + 18000 + random(15000);
        } else {
            static constexpr Mood tiny[] = {
                Mood::Blink, Mood::DoubleBlink, Mood::LookLeft, Mood::LookRight,
                Mood::LookUp, Mood::LookDown, Mood::WinkLeft, Mood::WinkRight,
                Mood::SideEye, Mood::Thinking, Mood::RollEyes, Mood::Peek
            };
            static constexpr Mood extra[] = {
                Mood::Happy, Mood::Laugh, Mood::Blush, Mood::Hearts,
                Mood::StarEyes, Mood::Sparkles, Mood::Surprise, Mood::Confused,
                Mood::Pout, Mood::Grumpy, Mood::Mischief, Mood::Tongue,
                Mood::Giggle, Mood::Party, Mood::Bounce, Mood::Dizzy,
                Mood::Shy, Mood::Yawn, Mood::Sing, Mood::Tears,
                Mood::Sleepy, Mood::Idea, Mood::Laser, Mood::Countdown,
                Mood::Bubbles, Mood::Rainbow, Mood::Peekaboo
            };
            const bool subtle = random(100) < 76;
            Mood next = subtle ? tiny[random(sizeof(tiny) / sizeof(tiny[0]))]
                               : extra[random(sizeof(extra) / sizeof(extra[0]))];
            if (next == previous_) next = Mood::Blink;
            previous_ = next;
            play(next, now, subtle ? 700 + random(650) : 1400 + random(1050));
            nextIdleAt_ = now + 6000 + random(12500);
        }
    }

    Mood mood() const { return current_; }
    bool sleeping() const { return sleeping_; }
    uint32_t elapsed(uint32_t now) const { return now - startedAt_; }
    uint32_t lastActivity() const { return lastInteraction_; }

private:
    uint32_t rng_;
    uint32_t lastInteraction_ = 0;
    uint32_t startedAt_ = 0;
    uint32_t activeUntil_ = 0;
    uint32_t nextIdleAt_ = 0;
    Mood current_ = Mood::Neutral;
    Mood previous_ = Mood::Neutral;
    bool sleeping_ = false;

    uint32_t random(uint32_t maxExclusive) {
        if (!maxExclusive) return 0;
        rng_ ^= rng_ << 13;
        rng_ ^= rng_ >> 17;
        rng_ ^= rng_ << 5;
        return rng_ % maxExclusive;
    }
    void play(Mood next, uint32_t now, uint32_t duration) {
        current_ = next;
        startedAt_ = now;
        activeUntil_ = now + duration;
        nextIdleAt_ = activeUntil_ + 6000 + random(10000);
    }
};
