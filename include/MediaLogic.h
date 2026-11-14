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

// Feedback for the action requested by the dial, not the computer's actual
// sound level. USB HID Consumer Control is one-way and cannot read back volume.
// Coalesce changes and discard stale unsent actions on direction reversal.
// Slow HID hosts can lag the knob; bounding the queue prevents multi-second
// unwanted volume changes after the user stops or changes direction.
class VolumeQueue {
public:
    explicit VolumeQueue(int limit) : limit_(limit > 0 ? limit : 1) {}
    void add(int signedCommands) {
        if (!signedCommands) return;
        if ((pending_ > 0 && signedCommands < 0) ||
            (pending_ < 0 && signedCommands > 0)) pending_ = 0;
        const int next = pending_ + signedCommands;
        pending_ = next > limit_ ? limit_ : next < -limit_ ? -limit_ : next;
    }
    int pending() const { return pending_; }
    int direction() const { return (pending_ > 0) - (pending_ < 0); }
    void sent() { pending_ -= direction(); }
    void reset() { pending_ = 0; }
private:
    int pending_ = 0;
    int limit_ = 1;
};

class VolumeFeedback {
public:
    void turn(int signedCommands, uint32_t now) {
        if (!signedCommands) return;
        direction_ = signedCommands > 0 ? 1 : -1;
        lastTurn_ = now;
        active_ = true;
    }
    void dismiss() { active_ = false; }
    int direction() const { return direction_; }
    bool visible(uint32_t now, uint32_t timeout) const {
        return active_ && (now - lastTurn_) < timeout;
    }
private:
    int direction_ = 0;
    uint32_t lastTurn_ = 0;
    bool active_ = false;
};

inline bool shouldAutoBuddy(uint32_t now, uint32_t lastActivity, bool isMedia, uint32_t timeout) {
    return isMedia && (now - lastActivity) >= timeout;
}
