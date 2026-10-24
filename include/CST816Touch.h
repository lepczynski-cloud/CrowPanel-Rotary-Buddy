#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"

struct TouchSample {
    bool pressed = false;
    bool released = false;
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t gesture = 0;
    uint8_t event = 3;
    uint8_t fingers = 0;
};

// CST816D touch controller used by the CrowPanel 1.28-inch rotary display.
// Runtime input is polled, as in Elecrow's reference driver. It does not rely
// on receiving one particular IRQ edge before a contact may start.
class CST816Touch {
public:
    bool begin() {
        contactActive_ = false;
        lastX_ = 120;
        lastY_ = 120;

        // Use the same default Wire controller and startup order as Elecrow's
        // reference implementation. No other I2C device is used by this app.
        if (!Wire.begin(Config::TOUCH_SDA, Config::TOUCH_SCL)) return false;
        Wire.setClock(Config::TOUCH_I2C_HZ);
        Wire.setTimeOut(30);

        pinMode(Config::TOUCH_IRQ, OUTPUT);
        digitalWrite(Config::TOUCH_IRQ, HIGH);
        delay(1);
        digitalWrite(Config::TOUCH_IRQ, LOW);
        delay(1);

        pinMode(Config::TOUCH_RESET, OUTPUT);
        digitalWrite(Config::TOUCH_RESET, LOW);
        delay(10);
        digitalWrite(Config::TOUCH_RESET, HIGH);
        delay(300);
        pinMode(Config::TOUCH_IRQ, INPUT_PULLUP);

        // A short retry window avoids treating a slow power-up as a permanent
        // touch failure.
        bool detected = false;
        for (int attempt = 0; attempt < 5 && !detected; ++attempt) {
            detected = probe();
            if (!detected) delay(20);
        }
        if (!detected) return false;

        // Disable automatic low-power mode, matching the published driver.
        if (!writeRegister(0xFE, 0xFF)) return false;
        delay(5);
        return true;
    }

    bool read(TouchSample &point) {
        uint8_t coordinates[4] = {};
        uint8_t fingerRegister = 0;

        // Match Elecrow's access pattern: individual finger/gesture reads use
        // a repeated start; the coordinate block uses a normal STOP first.
        if (!readByteRepeatedStart(0x02, fingerRegister) ||
            !readByteRepeatedStart(0x01, point.gesture) ||
            !readBlock(0x03, coordinates, sizeof(coordinates)))
            return false;

        point.fingers = fingerRegister & 0x0F;
        point.event = coordinates[0] >> 6;

        uint16_t x = ((coordinates[0] & 0x0F) << 8) | coordinates[1];
        uint16_t y = ((coordinates[2] & 0x0F) << 8) | coordinates[3];
        const bool touching = point.fingers > 0;

        if (touching) {
            if (x >= 240 || y >= 240) return false;
            transform(x, y);
            lastX_ = x;
            lastY_ = y;
            point.x = x;
            point.y = y;
            point.pressed = true;
            point.released = false;
        } else {
            // Coordinates can be stale after lift-up. Reuse the final valid
            // point so that a tap is hit-tested where the finger was released.
            point.x = lastX_;
            point.y = lastY_;
            point.pressed = false;
            point.released = contactActive_;
        }

        contactActive_ = touching;
        return true;
    }

private:
    static constexpr uint8_t ADDRESS = 0x15;
    bool contactActive_ = false;
    uint16_t lastX_ = 120;
    uint16_t lastY_ = 120;

    static bool probe() {
        Wire.beginTransmission(ADDRESS);
        return Wire.endTransmission(true) == 0;
    }

    static bool writeRegister(uint8_t reg, uint8_t value) {
        Wire.beginTransmission(ADDRESS);
        if (Wire.write(reg) != 1 || Wire.write(value) != 1) return false;
        return Wire.endTransmission(true) == 0;
    }

    static bool readByteRepeatedStart(uint8_t reg, uint8_t &value) {
        Wire.beginTransmission(ADDRESS);
        if (Wire.write(reg) != 1) return false;
        if (Wire.endTransmission(false) != 0) return false;
        if (Wire.requestFrom(ADDRESS, static_cast<uint8_t>(1),
                             static_cast<uint8_t>(true)) != 1)
            return false;
        value = static_cast<uint8_t>(Wire.read());
        return true;
    }

    static bool readBlock(uint8_t reg, uint8_t *buffer, size_t length) {
        Wire.beginTransmission(ADDRESS);
        if (Wire.write(reg) != 1) return false;
        if (Wire.endTransmission(true) != 0) return false;
        const uint8_t received = Wire.requestFrom(
            ADDRESS, static_cast<uint8_t>(length), static_cast<uint8_t>(true));
        if (received != length) return false;
        for (size_t i = 0; i < length; ++i)
            buffer[i] = static_cast<uint8_t>(Wire.read());
        return true;
    }

    static void transform(uint16_t &x, uint16_t &y) {
        if (Config::TOUCH_SWAP_XY) {
            const uint16_t oldX = x;
            x = y;
            y = oldX;
        }
        if (Config::TOUCH_INVERT_X) x = 239 - x;
        if (Config::TOUCH_INVERT_Y) y = 239 - y;
    }
};
