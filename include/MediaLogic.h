#pragma once
#include <stdint.h>

// Returns IDs for the FOUR large media touch zones. Unrecognized touches are
// ignored, including swipes. Avoid tiny tap targets on a 240x240 circle.
enum class MediaButton : int8_t { None = -1, Previous = 0, PlayPause = 1, Next = 2, Mute = 3 };

inline MediaButton mediaHitTest(int x, int y) {
    const int prevX = x - 47, nextX = x - 193, playX = x - 120;
    const int dy = y - 119;
    if (playX * playX + dy * dy <= 37 * 37) return MediaButton::PlayPause;
    if (prevX * prevX + dy * dy <= 31 * 31) return MediaButton::Previous;
    if (nextX * nextX + dy * dy <= 31 * 31) return MediaButton::Next;
    const int muteX = x - 120, muteY = y - 188;
    if (muteX * muteX + muteY * muteY <= 30 * 30) return MediaButton::Mute;
    return MediaButton::None;
}

class VolumeIndicator {
public:
    explicit VolumeIndicator(int initial, int percentPerReport)
        : target_(clamp(initial)), displayed_(target_), step_(percentPerReport) {}
    static int clamp(int n) { return n < 0 ? 0 : n > 100 ? 100 : n; }
    void report(int reports, uint32_t now) {
        if (!reports) return;
        target_ = clamp(target_ + reports * step_);
        lastReport_ = now;
        active_ = true;
    }
    void update() {
        const int diff = target_ - displayed_;
        if (!diff) return;
        int amount = diff > 0 ? diff : -diff;
        amount = (amount + 3) / 4;
        displayed_ += diff > 0 ? amount : -amount;
    }
    void dismiss() { active_ = false; }
    int target() const { return target_; }
    int displayed() const { return displayed_; }
    bool visible(uint32_t now, uint32_t timeout) const {
        return active_ && (now - lastReport_) < timeout;
    }
private:
    int target_;
    int displayed_;
    int step_;
    uint32_t lastReport_ = 0;
    bool active_ = false;
};

inline bool shouldAutoBuddy(uint32_t now, uint32_t lastActivity, bool isMedia, uint32_t timeout) {
    return isMedia && (now - lastActivity) >= timeout;
}
