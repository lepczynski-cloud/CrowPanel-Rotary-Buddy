#pragma once
#include <stdint.h>

// Hardware-agnostic Gray-code quadrature decoder.
// Each stable detent normally yields 4 valid Gray-code transitions.
class RotaryDecoder {
public:
    explicit RotaryDecoder(uint8_t edgesPerStep = 4)
        : edgesPerStep_(edgesPerStep == 2 ? 2 : 4) {}

    void reset(bool a, bool b) {
        last_ = (static_cast<uint8_t>(a) << 1) | static_cast<uint8_t>(b);
        accumulated_ = 0;
    }

    // Returns +1, -1 or 0. Invalid/bounced two-bit jumps count as 0.
    int update(bool a, bool b) {
        static constexpr int8_t transitions[16] = {
             0, -1, +1,  0,
            +1,  0,  0, -1,
            -1,  0,  0, +1,
             0, +1, -1,  0
        };
        const uint8_t now = (static_cast<uint8_t>(a) << 1) | static_cast<uint8_t>(b);
        const uint8_t index = (last_ << 2) | now;
        last_ = now;
        accumulated_ += transitions[index];
        if (accumulated_ >= edgesPerStep_) {
            accumulated_ = 0;
            return 1;
        }
        if (accumulated_ <= -edgesPerStep_) {
            accumulated_ = 0;
            return -1;
        }
        return 0;
    }

private:
    uint8_t last_ = 0;
    int8_t accumulated_ = 0;
    uint8_t edgesPerStep_ = 4;
};
