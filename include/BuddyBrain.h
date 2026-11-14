#pragma once
#include <stdint.h>

// A small deterministic, offline animation scheduler.
// Reactions and captions are playful visuals only: no networking, listening,
// deployment commands or host keyboard shortcuts are triggered by the pet.
enum class Mood : uint8_t {
    Neutral,
    Blink, DoubleBlink, LookLeft, LookRight, LookUp, LookDown,
    WinkLeft, WinkRight, Happy, Laugh, Blush, Hearts, StarEyes,
    Sparkles, Surprise, Confused, Thinking, SideEye, Pout,
    Grumpy, Mischief, Tongue, Giggle, Party, Bounce, Dizzy,
    Peek, Shy, Yawn, Sing, Tears, Sleepy, RollEyes, Idea,
    Laser, Countdown, Bubbles, Rainbow, Peekaboo, Wake, Sleep, Dream,
    Wave, Squint, Glance, BigGrin, Hiccup, Sneeze, Victory, Disco,
    Confetti, Orbit, Fireworks, PixelRain, CodeRain, Rocket,
    Comet, CloudFloat, Rainy, Snowfall, Wiggle, Stretch, Boop,
    Glitch, Heartbeat, Loading, Deploy, Build, BugHunt, Coffee,
    Snack, Debug, HighFive, Panic, Weekend, Rollback, SleepTalk,
    Ping, Typing, Freeze, Zoomies, Noodle, Hypno, Magnet,
    PetBliss, Purr, Nuzzle, HeadTilt, HappyDance, Curious,
    Cap, Beanie, TopHat, WizardHat, Headphones, Crown, PartyHat,
    SleepCap, Sunglasses, Whistle, HelloWave, PeekScreen, Bashful,
    Alien, SideShuffle, Wifi,
    Prompting, CodeReview, OnCall, PacketLoss, ShipIt, CloudOps,
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
        "Roll eyes", "Idea", "Laser", "Countdown", "Bubbles", "Rainbow",
        "Peekaboo", "Wake", "Sleep", "Dream", "Wave", "Squint",
        "Glance", "Big grin", "Hiccup", "Sneeze", "Victory", "Disco",
        "Confetti", "Orbit", "Fireworks", "Pixel rain", "Code rain",
        "Rocket", "Comet", "Cloud float", "Rainy", "Snowfall", "Wiggle",
        "Stretch", "Boop", "Glitch", "Heartbeat", "Loading", "Deploy",
        "Build", "Bug hunt", "Coffee", "Snack", "Debug", "High five",
        "Panic", "Weekend", "Rollback", "Sleep talk", "Ping", "Typing",
        "Freeze", "Zoomies", "Noodle", "Hypno", "Magnet",
        "Pet bliss", "Purr", "Nuzzle", "Head tilt", "Happy dance", "Curious",
        "Cap", "Beanie", "Top hat", "Wizard hat", "Headphones", "Crown",
        "Party hat", "Sleep cap", "Sunglasses", "Whistle", "Hello wave",
        "Peek screen", "Bashful", "Alien", "Side shuffle", "Wi-Fi",
        "AI prompting", "Code review", "On call", "Packet loss", "Ship it", "Cloud ops"
    };
    constexpr unsigned n = sizeof(names) / sizeof(names[0]);
    static_assert(n == BUDDY_MOOD_COUNT, "Mood descriptions must match the enum");
    const unsigned index = static_cast<unsigned>(mood);
    return index < n ? names[index] : "Unknown";
}

struct BuddyQuip {
    Mood mood;
    const char *line;
};

// Short, original captions sized for the 240x240 round display. All of these
// are visual jokes: e.g. "GO TO PROD!" does NOT deploy any software.
static constexpr BuddyQuip BUDDY_QUIPS[] = {
    {Mood::Deploy, "LET'S GO PROD!"}, {Mood::Deploy, "SHIP TO PROD?"},
    {Mood::Deploy, "DEPLOY TIME!"}, {Mood::Deploy, "DO IT LIVE?"},
    {Mood::Build, "BUILD IS GREEN!"}, {Mood::Build, "CI SAYS YES!"},
    {Mood::Build, "ONE MORE BUILD"}, {Mood::Build, "IT COMPILES!"},
    {Mood::BugHunt, "BUG? FEATURE!"}, {Mood::BugHunt, "I SEE BUGS"},
    {Mood::BugHunt, "NOT MY BUG"}, {Mood::BugHunt, "WHERE'S THE BUG?"},
    {Mood::Coffee, "COFFEE FIRST"}, {Mood::Coffee, "NEED MORE JAVA"},
    {Mood::Coffee, "FUEL: COFFEE"}, {Mood::Coffee, "404: COFFEE"},
    {Mood::Snack, "SNACK BREAK?"}, {Mood::Snack, "EAT. CODE. NAP."},
    {Mood::Debug, "WORKS ON MINE!"}, {Mood::Debug, "CHECK THE LOGS"},
    {Mood::Debug, "IS IT DNS?"}, {Mood::Debug, "TRY TURNING..."},
    {Mood::Victory, "LGTM!"}, {Mood::Victory, "PR APPROVED!"},
    {Mood::Victory, "YOU DID IT!"}, {Mood::Victory, "NICE COMMIT!"},
    {Mood::Panic, "EVERYTHING'S FINE"}, {Mood::Panic, "NO PANIC!"},
    {Mood::Panic, "ALL SYSTEMS GO"}, {Mood::Weekend, "IS IT FRIDAY?"},
    {Mood::Weekend, "FRIDAY DEPLOY?"}, {Mood::Weekend, "WEEKEND MODE!"},
    {Mood::Rollback, "ROLLBACK?"}, {Mood::Rollback, "CTRL+Z PLEASE"},
    {Mood::Rollback, "OOPS... UNDO"}, {Mood::HighFive, "HIGH FIVE!"},
    {Mood::HighFive, "TEAMWORK!"}, {Mood::Loading, "THINKING..."},
    {Mood::Loading, "LOADING BRAIN"}, {Mood::Loading, "99%... ALWAYS"},
    {Mood::Glitch, "404: BRAIN"}, {Mood::Glitch, "BEEP... BOOP?"},
    {Mood::Glitch, "HELLO, ERROR"}, {Mood::Rocket, "TO THE CLOUD!"},
    {Mood::Rocket, "CLOUD NINE!"}, {Mood::CodeRain, "HELLO, WORLD!"},
    {Mood::CodeRain, "0101010101"}, {Mood::Typing, "GIT PUSH!"},
    {Mood::Typing, "ONE MORE PR"}, {Mood::Ping, "PING! PONG!"},
    {Mood::Ping, "STILL ONLINE!"}, {Mood::Freeze, "AFK... BRB"},
    {Mood::Freeze, "I'M ON BREAK"}, {Mood::Disco, "DEBUG DANCE!"},
    {Mood::Disco, "PARTY IN RAM!"}, {Mood::Confetti, "TA-DA!"},
    {Mood::Hiccup, "HIC!"}, {Mood::Sneeze, "ACHOO!"},
    {Mood::Stretch, "STRETCH BREAK!"}, {Mood::BigGrin, "SMILE.EXE"},
    {Mood::Boop, "BOOP!"}, {Mood::Orbit, "WHEEEE!"},
    {Mood::Noodle, "NOODLE MODE!"}, {Mood::Magnet, "ATTRACTING BUGS"},
    {Mood::Zoomies, "ZOOM ZOOM!"}, {Mood::Hypno, "LOOK AT ME!"},
    {Mood::CloudFloat, "CLOUD WATCH"}, {Mood::Rainy, "RAIN AGAIN?"},
    {Mood::PixelRain, "PIXEL PARTY!"}, {Mood::Comet, "I AM SPEED"},
    {Mood::Wiggle, "DANCE WITH ME"}, {Mood::SleepTalk, "Zzz... PROD"},
    {Mood::HelloWave, "HEY, YOU!"}, {Mood::HelloWave, "HI, HUMAN!"},
    {Mood::HelloWave, "GOOD TO SEE YOU"}, {Mood::HelloWave, "WHAT'S UP?"},
    {Mood::Curious, "WHAT ARE YOU DOING?"}, {Mood::Curious, "BUSY TODAY?"},
    {Mood::Curious, "GOT ANY PLANS?"}, {Mood::Curious, "CAN I HELP?"},
    {Mood::PeekScreen, "I SEE YOU!"}, {Mood::PeekScreen, "PSST... HI!"},
    {Mood::HeadTilt, "TELL ME MORE!"}, {Mood::HeadTilt, "HMM?"},
    {Mood::Cap, "CAPTAIN BUDDY!"}, {Mood::Cap, "HAT CHECK!"},
    {Mood::Beanie, "COZY MODE!"}, {Mood::TopHat, "VERY FANCY!"},
    {Mood::WizardHat, "CASTING SMILES!"}, {Mood::Headphones, "GOT THE BEAT!"},
    {Mood::Headphones, "DJ BUDDY!"}, {Mood::Crown, "YOUR MAJESTY!"},
    {Mood::PartyHat, "PARTY TIME!"}, {Mood::SleepCap, "FIVE MORE MINS"},
    {Mood::Sunglasses, "TOO COOL!"}, {Mood::Whistle, "WHISTLE WHILE..."},
    {Mood::HappyDance, "DANCE BREAK!"}, {Mood::Bashful, "AW, SHUCKS!"},
    {Mood::Alien, "HELLO, EARTHLING"}, {Mood::Wifi, "IS IT THE WI-FI?"},
    {Mood::SideShuffle, "JUST VIBING!"}, {Mood::Nuzzle, "A LITTLE CUDDLE?"},
    {Mood::PetBliss, "MORE PETS!"}, {Mood::Purr, "PURRR..."},
    {Mood::Coffee, "BREWING IDEAS"}, {Mood::Coffee, "COFFEE.EXE"},
    {Mood::Debug, "IT'S ALWAYS DNS"}, {Mood::Weekend, "NO MEETINGS!"},
    // Engineering humour only: Buddy has no AI model, internet connection,
    // production credentials, remote execution or pager integration.
    {Mood::Prompting, "ASK THE MODEL"}, {Mood::Prompting, "VERIFY THE OUTPUT"},
    {Mood::Prompting, "HUMAN IN THE LOOP"}, {Mood::Prompting, "TOKEN BUDGET?"},
    {Mood::Prompting, "PROMPT. TEST. REPEAT."},
    {Mood::CodeReview, "CHECK THE DIFF"}, {Mood::CodeReview, "LGTM... MAYBE"},
    {Mood::CodeReview, "REVIEW BEFORE MERGE"}, {Mood::CodeReview, "ONE MORE COMMENT"},
    {Mood::CodeReview, "DID YOU TEST IT?"},
    {Mood::OnCall, "PAGER AGAIN?"}, {Mood::OnCall, "3 AM ALERT!"},
    {Mood::OnCall, "ON-CALL MODE"}, {Mood::OnCall, "PLEASE BE GREEN"},
    {Mood::OnCall, "NO NEW ALERTS?"},
    {Mood::PacketLoss, "PACKET LOST!"}, {Mood::PacketLoss, "RETRYING..."},
    {Mood::PacketLoss, "WHERE'S MY PACKET?"}, {Mood::PacketLoss, "NETWORK GREMLINS"},
    {Mood::ShipIt, "SHIP IT SAFELY"}, {Mood::ShipIt, "ROLLBACK READY?"},
    {Mood::ShipIt, "GREEN PIPELINE!"}, {Mood::ShipIt, "CANARY FIRST"},
    {Mood::CloudOps, "CHECK THE METRICS"}, {Mood::CloudOps, "CLOUD BILL AGAIN"},
    {Mood::CloudOps, "COSTS ARE REAL"}, {Mood::CloudOps, "SCALE DOWN?"}
};

constexpr unsigned BUDDY_QUIP_COUNT = sizeof(BUDDY_QUIPS) / sizeof(BUDDY_QUIPS[0]);

class BuddyBrain {
public:
    explicit BuddyBrain(uint32_t seed = 0x91A73BD2u) : rng_(seed ? seed : 1u) {}

    void begin(uint32_t now) {
        lastInteraction_ = now;
        startedAt_ = now;
        current_ = Mood::Wake;
        activeQuip_ = "HELLO, HUMAN!";
        activeUntil_ = now + 1400;
        nextIdleAt_ = now + 8000;
        sleeping_ = false;
        previousQuipIndex_ = 0xffffu;
        lastPetLine_ = 0xffu;
    }

    void activity(uint32_t now, bool playful = false) {
        const bool wasSleeping = sleeping_;
        sleeping_ = false;
        lastInteraction_ = now;
        if (wasSleeping) {
            play(Mood::Wake, now, 1600, "I'M AWAKE!");
        } else if (playful) {
            if (random(100) < 55) {
                const BuddyQuip &quip = nextQuip();
                play(quip.mood, now, 1900 + random(1150), quip.line);
            } else {
                static constexpr Mood reactions[] = {
                    Mood::Happy, Mood::Laugh, Mood::Blush, Mood::Hearts,
                    Mood::StarEyes, Mood::Surprise, Mood::Pout, Mood::Tongue,
                    Mood::Giggle, Mood::Party, Mood::Dizzy, Mood::Shy,
                    Mood::Laser, Mood::Peekaboo, Mood::Idea, Mood::Bubbles,
                    Mood::Rainbow, Mood::Mischief, Mood::Victory, Mood::Disco,
                    Mood::Confetti, Mood::Rocket, Mood::Wave, Mood::Glitch,
                    Mood::Fireworks, Mood::Zoomies, Mood::Wiggle, Mood::Boop,
                    Mood::Curious, Mood::HelloWave, Mood::HeadTilt,
                    Mood::Crown, Mood::TopHat, Mood::Beanie, Mood::Headphones,
                    Mood::Prompting, Mood::CodeReview, Mood::ShipIt
                };
                constexpr uint32_t count = sizeof(reactions) / sizeof(reactions[0]);
                const Mood next = reactions[random(count)];
                play(next, now, 1500 + random(1200));
            }
        }
    }

    // Stroke down the touchscreen to pet Buddy. Never emits any host HID key.
    void pet(uint32_t now) {
        sleeping_ = false;
        lastInteraction_ = now;
        // A gentle, continuous left-to-right closed-eye sway. Unlike the old
        // random reaction, every stroke feels like the same friendly gesture.
        static constexpr const char *petLines[] = {
            "THAT FEELS NICE!", "PURRR...", "THANK YOU!",
            "PET ME AGAIN!", "BEST HUMAN!", "I LIKE YOU!"
        };
        constexpr unsigned count = sizeof(petLines) / sizeof(petLines[0]);
        unsigned choice = random(lastPetLine_ < count ? count - 1 : count);
        if (lastPetLine_ < count && choice >= lastPetLine_) ++choice;
        lastPetLine_ = static_cast<uint8_t>(choice);
        play(Mood::PetBliss, now, 2300, petLines[choice]);
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
            activeQuip_ = nullptr;
            startedAt_ = now;
            nextIdleAt_ = now + (sleeping_ ? 16000 + random(12000) : 4800 + random(8400));
        }
        if (static_cast<int32_t>(now - nextIdleAt_) < 0) return;
        if (sleeping_) {
            if (random(100) < 18) {
                play(Mood::SleepTalk, now, 2100, "Zzz... PROD");
            } else {
                play(Mood::Dream, now, 1800 + random(1100));
            }
            nextIdleAt_ = now + 18000 + random(15000);
        } else {
            static constexpr Mood subtle[] = {
                Mood::Blink, Mood::DoubleBlink, Mood::LookLeft, Mood::LookRight,
                Mood::LookUp, Mood::LookDown, Mood::WinkLeft, Mood::WinkRight,
                Mood::SideEye, Mood::Thinking, Mood::RollEyes, Mood::Peek,
                Mood::Squint, Mood::Glance, Mood::Wiggle, Mood::Stretch,
                Mood::Ping, Mood::Typing, Mood::Freeze, Mood::CloudFloat,
                Mood::CodeReview, Mood::OnCall
            };
            static constexpr Mood expressive[] = {
                Mood::Happy, Mood::Laugh, Mood::Blush, Mood::Hearts,
                Mood::StarEyes, Mood::Sparkles, Mood::Surprise, Mood::Confused,
                Mood::Pout, Mood::Grumpy, Mood::Mischief, Mood::Tongue,
                Mood::Giggle, Mood::Party, Mood::Bounce, Mood::Dizzy,
                Mood::Shy, Mood::Yawn, Mood::Sing, Mood::Tears,
                Mood::Sleepy, Mood::Idea, Mood::Laser, Mood::Countdown,
                Mood::Bubbles, Mood::Rainbow, Mood::Peekaboo,
                Mood::Wave, Mood::BigGrin, Mood::Hiccup, Mood::Sneeze,
                Mood::Victory, Mood::Disco, Mood::Confetti, Mood::Orbit,
                Mood::Fireworks, Mood::PixelRain, Mood::CodeRain, Mood::Rocket,
                Mood::Comet, Mood::CloudFloat, Mood::Rainy, Mood::Snowfall,
                Mood::Wiggle, Mood::Stretch, Mood::Boop, Mood::Glitch,
                Mood::Heartbeat, Mood::Loading, Mood::Deploy, Mood::Build,
                Mood::BugHunt, Mood::Coffee, Mood::Snack, Mood::Debug,
                Mood::HighFive, Mood::Panic, Mood::Weekend, Mood::Rollback,
                Mood::Ping, Mood::Typing, Mood::Freeze, Mood::Zoomies,
                Mood::Noodle, Mood::Hypno, Mood::Magnet,
                Mood::Purr, Mood::Nuzzle, Mood::HeadTilt,
                Mood::HappyDance, Mood::Curious, Mood::Cap, Mood::Beanie,
                Mood::TopHat, Mood::WizardHat, Mood::Headphones, Mood::Crown,
                Mood::PartyHat, Mood::SleepCap, Mood::Sunglasses, Mood::Whistle,
                Mood::HelloWave, Mood::PeekScreen, Mood::Bashful, Mood::Alien,
                Mood::SideShuffle, Mood::Wifi, Mood::Prompting,
                Mood::PacketLoss, Mood::ShipIt, Mood::CloudOps
            };
            const uint32_t roll = random(100);
            if (roll < 70) {
                const Mood next = subtle[random(sizeof(subtle) / sizeof(subtle[0]))];
                play(next, now, 750 + random(750));
            } else if (roll < 86) {
                const Mood next = expressive[random(sizeof(expressive) / sizeof(expressive[0]))];
                play(next == previous_ ? Mood::Blink : next, now, 1400 + random(1050));
            } else {
                const BuddyQuip &quip = nextQuip();
                play(quip.mood, now, 2050 + random(950), quip.line);
            }
            previous_ = current_;
            nextIdleAt_ = now + 6000 + random(12500);
        }
    }

    Mood mood() const { return current_; }
    bool sleeping() const { return sleeping_; }
    const char *quip() const { return activeQuip_; }
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
    const char *activeQuip_ = nullptr;
    bool sleeping_ = false;
    uint16_t previousQuipIndex_ = 0xffffu;
    uint8_t lastPetLine_ = 0xffu;

    // Uniform selection excluding the immediately previous caption.
    // Prevents a distracting duplicate when repeatedly tapping Buddy.
    const BuddyQuip &nextQuip() {
        const unsigned count = BUDDY_QUIP_COUNT;
        if (count <= 1) return BUDDY_QUIPS[0];
        unsigned pick = random(previousQuipIndex_ < count ? count - 1 : count);
        if (previousQuipIndex_ < count && pick >= previousQuipIndex_) ++pick;
        previousQuipIndex_ = static_cast<uint16_t>(pick);
        return BUDDY_QUIPS[pick];
    }

    uint32_t random(uint32_t maxExclusive) {
        if (!maxExclusive) return 0;
        rng_ ^= rng_ << 13;
        rng_ ^= rng_ >> 17;
        rng_ ^= rng_ << 5;
        return rng_ % maxExclusive;
    }

    void play(Mood next, uint32_t now, uint32_t duration, const char *caption = nullptr) {
        current_ = next;
        activeQuip_ = caption;
        startedAt_ = now;
        activeUntil_ = now + duration;
        nextIdleAt_ = activeUntil_ + 6000 + random(10000);
    }
};
