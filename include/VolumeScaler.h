#pragma once

// Divides signed pulse counts while carrying the fractional remainder between
// detents. Direction changes discard the old-direction remainder immediately.
class VolumeScaler {
public:
    explicit VolumeScaler(int divisor) : divisor_(divisor > 0 ? divisor : 1) {}

    int apply(int signedPulses) {
        if (!signedPulses) return 0;
        const int direction = signedPulses > 0 ? 1 : -1;
        if (lastDirection_ && direction != lastDirection_) remainder_ = 0;
        lastDirection_ = direction;
        remainder_ += signedPulses;
        const int output = remainder_ / divisor_;
        remainder_ -= output * divisor_;
        return output;
    }

    void reset() {
        remainder_ = 0;
        lastDirection_ = 0;
    }

private:
    int divisor_ = 1;
    int remainder_ = 0;
    int lastDirection_ = 0;
};
