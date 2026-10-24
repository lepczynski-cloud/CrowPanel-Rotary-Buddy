#pragma once
#include <stdint.h>
#include <stdlib.h>

// Hardware-independent touch classifier. Each contact can produce either one
// swipe OR one tap, never both. Horizontal swipes change screens,
// a downward swipe pets Buddy, and an upward swipe is intentionally ignored. A swipe fires as soon as motion is sufficient,
// so it still works if some CST816D revisions miss the lift-up interrupt.
enum class GestureType : uint8_t { None, Tap, SwipeLeft, SwipeRight, SwipeUp, PetDown };

struct GestureResult {
    GestureType type = GestureType::None;
    int startX = 0;
    int startY = 0;
    int endX = 0;
    int endY = 0;
};

class TouchGesture {
public:
    bool active() const { return active_; }
    void reset() {
        active_ = false;
        tooFarForTap_ = false;
        firedSwipe_ = false;
    }

    // CST816D IDs: 1 down, 2 up, 3 left, 4 right.
    static GestureType hardwareDirection(uint8_t id) {
        if (id == 1) return GestureType::PetDown;
        if (id == 2) return GestureType::SwipeUp;
        if (id == 3) return GestureType::SwipeLeft;
        if (id == 4) return GestureType::SwipeRight;
        return GestureType::None;
    }

    GestureResult consume(bool pressed, bool released, uint8_t hardwareGesture,
                          int x, int y, unsigned long now,
                          unsigned long tapMinMs, unsigned long tapMaxMs,
                          int tapSlopPx, int minSwipePx) {
        GestureResult result;
        if (pressed) {
            if (!active_) {
                active_ = true;
                startX_ = lastX_ = x;
                startY_ = lastY_ = y;
                startMs_ = now;
                tooFarForTap_ = false;
                firedSwipe_ = false;
            }
            lastX_ = x;
            lastY_ = y;
            const int dx = lastX_ - startX_;
            const int dy = lastY_ - startY_;
            if (abs(dx) > tapSlopPx || abs(dy) > tapSlopPx) tooFarForTap_ = true;
            if (!firedSwipe_ && now - startMs_ <= 1100) {
                if (abs(dx) >= minSwipePx && abs(dx) * 4 >= abs(dy) * 5)
                    result.type = dx < 0 ? GestureType::SwipeLeft : GestureType::SwipeRight;
                else if (abs(dy) >= minSwipePx && abs(dy) * 4 >= abs(dx) * 5)
                    result.type = dy < 0 ? GestureType::SwipeUp : GestureType::PetDown;
                else {
                    // The controller's direction hint may be stale. Only accept
                    // it if measured movement agrees with the hinted axis.
                    const GestureType hinted = hardwareDirection(hardwareGesture);
                    const int fallbackMin = (minSwipePx * 2) / 3;
                    if ((hinted == GestureType::SwipeLeft && dx <= -fallbackMin && abs(dx) > abs(dy)) ||
                        (hinted == GestureType::SwipeRight && dx >= fallbackMin && abs(dx) > abs(dy)) ||
                        (hinted == GestureType::PetDown && dy >= fallbackMin && abs(dy) > abs(dx)) ||
                        (hinted == GestureType::SwipeUp && dy <= -fallbackMin && abs(dy) > abs(dx)))
                        result.type = hinted;
                }
                if (result.type != GestureType::None) {
                    firedSwipe_ = true;
                    tooFarForTap_ = true;
                }
            }
        }
        result.startX = startX_;
        result.startY = startY_;
        result.endX = lastX_;
        result.endY = lastY_;
        if (released && active_) {
            if (!firedSwipe_ && !tooFarForTap_ &&
                now - startMs_ >= tapMinMs && now - startMs_ <= tapMaxMs)
                result.type = GestureType::Tap;
            reset();
        }
        return result;
    }

private:
    bool active_ = false;
    bool tooFarForTap_ = false;
    bool firedSwipe_ = false;
    int startX_ = 0;
    int startY_ = 0;
    int lastX_ = 0;
    int lastY_ = 0;
    unsigned long startMs_ = 0;
};
