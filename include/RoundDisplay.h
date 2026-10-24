#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <Arduino.h>
#include "Config.h"

// Pins and inversion adapted from the manufacturer's published GC9A01 config.
class RoundDisplay : public lgfx::LGFX_Device {
public:
    RoundDisplay() {
        {
            auto cfg = bus_.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = Config::LCD_SCLK;
            cfg.pin_mosi = Config::LCD_MOSI;
            cfg.pin_miso = -1;
            cfg.pin_dc = Config::LCD_DC;
            bus_.config(cfg);
            panel_.setBus(&bus_);
        }
        {
            auto cfg = panel_.config();
            cfg.pin_cs = Config::LCD_CS;
            cfg.pin_rst = Config::LCD_RESET;
            cfg.pin_busy = -1;
            cfg.panel_width = 240;
            cfg.panel_height = 240;
            cfg.memory_width = 240;
            cfg.memory_height = 240;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            panel_.config(cfg);
        }
        setPanel(&panel_);
    }

private:
    lgfx::Panel_GC9A01 panel_;
    lgfx::Bus_SPI bus_;
};
