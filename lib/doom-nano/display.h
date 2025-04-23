/*
todo: Moving this to CPP looks like it takes more Flash storage. Figure out why.
*/
#include "constants.h"
#include "sprites.h"
#include <Arduino.h>
#include <etl/algorithm.h>
#include <sdl/sdl_like.hpp>

// Reads a char from an F() string
#define F_char(ifsh, ch) pgm_read_byte(reinterpret_cast<PGM_P>(ifsh) + ch)

// This is slightly faster than bitRead (also bits are read from left to right)
const static uint8_t PROGMEM bit_mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
#define read_bit(b, n) b &pgm_read_byte(bit_mask + n) ? 1 : 0

void setupDisplay();
bool getGradientPixel(uint8_t x, uint8_t y, uint8_t i);
void fadeScreen(uint8_t intensity, bool color);
void drawByte(uint8_t x, uint8_t y, uint8_t b);
uint8_t getByte(uint8_t x, uint8_t y);
void drawPixel(int8_t x, int8_t y, bool color, bool raycasterViewport);
void drawVLine(uint8_t x, int8_t start_y, int8_t end_y, uint8_t intensity);
void drawSprite(int8_t x, int8_t y, const uint8_t bitmap[],
                const uint8_t mask[], int16_t w, int16_t h, uint8_t sprite,
                double distance);
void drawSprite(int8_t x, int8_t y, const uint16_t bitmap[], int16_t w,
                int16_t h, double distance);
void drawText(int8_t x, int8_t y, const char *txt,
              sdl::Color color = sdl::Color::kWhite);
void drawBitmap(int8_t x, int8_t y, const uint8_t *data, int16_t w, int16_t h,
                int16_t color);

// Initialize screen. Following line is for OLED 128x64 connected by I2C
// Adafruit_SSD1306<SCREEN_WIDTH, SCREEN_HEIGHT> display;

double delta = 1;

// #ifdef OPTIMIZE_SSD1306
// // Optimizations for SSD1306 handles buffer directly
// uint8_t *display_buf;
// #endif

// We don't handle more than MAX_RENDER_DEPTH depth, so we can safety store
// z values in a byte with 1 decimal and save some memory,
uint8_t zbuffer[ZBUFFER_SIZE];

void setupDisplay() {
  // initialize z buffer
  memset(zbuffer, 0xFF, ZBUFFER_SIZE);
}

void drawByte(uint8_t x, uint8_t y, uint8_t b) {
  for (size_t idx = 1; idx < 9; ++idx) {
    sdl::Sprite()->drawPixel(x, y + idx,
                             (b & ~idx) ? sdl::kWhite : sdl::kBlack);
  }
}

bool getGradientPixel(uint8_t x, uint8_t y, uint8_t i) {
  if (i == 0)
    return 0;
  if (i >= GRADIENT_COUNT - 1)
    return 1;

  uint8_t index = etl::max(0, etl::min(GRADIENT_COUNT - 1, (int)i)) *
                      GRADIENT_WIDTH * GRADIENT_HEIGHT // gradient index
                  + y * GRADIENT_WIDTH % (GRADIENT_WIDTH * GRADIENT_HEIGHT)
                  // y byte offset
                  + x / GRADIENT_HEIGHT % GRADIENT_WIDTH; // x byte offset

  // return the bit based on x
  return read_bit(pgm_read_byte(gradient + index), x % 8);
}

void fadeScreen(uint8_t intensity, bool color = 0) {
  // for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
  //   for (uint8_t y = 0; y < SCREEN_HEIGHT; y++) {
  //     if (getGradientPixel(x, y, intensity))
  //       drawPixel(x, y, color, false);
  //   }
  // }
}

// Faster drawPixel than display.drawPixel.
// Avoids some checks to make it faster.
void drawPixel(int8_t x, int8_t y, bool color, bool raycasterViewport = false) {
  // prevent write out of screen buffer
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 ||
      y >= (raycasterViewport ? RENDER_HEIGHT : SCREEN_HEIGHT)) {
    return;
  }

  sdl::u16 pixelColor = color ? sdl::kWhite : sdl::kBlack;
  sdl::Sprite()->drawPixel(x, y, pixelColor);
}

// For raycaster only
// Custom draw Vertical lines that fills with a pattern to simulate
// different brightness. Affected by RES_DIVIDER
void drawVLine(uint8_t x, int8_t start_y, int8_t end_y, uint8_t intensity) {
  int8_t y;
  int8_t lower_y = etl::max(etl::min(start_y, end_y), (int8_t)0);
  int8_t higher_y = etl::min(etl::max(start_y, end_y),
                             static_cast<int8_t>(RENDER_HEIGHT - 1));
  uint8_t c;

  uint8_t bp;
  uint8_t b;
  for (c = 0; c < RES_DIVIDER; c++) {
    y = lower_y;
    b = 0;
    while (y <= higher_y) {
      bp = y % 8;
      b = b | getGradientPixel(x + c, y, intensity) << bp;

      if (bp == 7) {
        // write the whole byte
        drawByte(x + c, y, b);
        b = 0;
      }

      y++;
    }

    // draw last byte
    if (bp != 7) {
      drawByte(x + c, y - 1, b);
    }
  }
}

// Custom drawBitmap method with scale support, mask, zindex and pattern filling
void drawSprite(int8_t x, int8_t y, const uint8_t bitmap[],
                const uint8_t mask[], int16_t w, int16_t h, uint8_t sprite,
                double distance) {
  uint8_t tw = (double)w / distance;
  uint8_t th = (double)h / distance;
  uint8_t byte_width = w / 8;
  uint8_t pixel_size = etl::max(1.0, 1.0 / distance);
  uint16_t sprite_offset = byte_width * h * sprite;

  bool pixel;
  bool maskPixel;

  // Don't draw the whole sprite if the anchor is hidden by z buffer
  // Not checked per pixel for performance reasons
  if (zbuffer[etl::min(etl::max((int)x, 0), ZBUFFER_SIZE - 1) / Z_RES_DIVIDER] <
      distance * DISTANCE_MULTIPLIER) {
    return;
  }

  for (uint8_t ty = 0; ty < th; ty += pixel_size) {
    // Don't draw out of screen
    if (y + ty < 0 || y + ty >= RENDER_HEIGHT) {
      continue;
    }

    uint8_t sy = ty * distance; // The y from the sprite

    for (uint8_t tx = 0; tx < tw; tx += pixel_size) {
      uint8_t sx = tx * distance; // The x from the sprite
      uint16_t byte_offset = sprite_offset + sy * byte_width + sx / 8;

      // Don't draw out of screen
      if (x + tx < 0 || x + tx >= SCREEN_WIDTH) {
        continue;
      }

      maskPixel = read_bit(pgm_read_byte(mask + byte_offset), sx % 8);

      if (maskPixel) {
        pixel = read_bit(pgm_read_byte(bitmap + byte_offset), sx % 8);
        for (uint8_t ox = 0; ox < pixel_size; ox++) {
          for (uint8_t oy = 0; oy < pixel_size; oy++) {
            drawPixel(x + tx + ox, y + ty + oy, pixel, true);
          }
        }
      }
    }
  }
}

void drawSprite(int8_t x, int8_t y, const uint16_t bitmap[], int16_t w,
                int16_t h, double distance) {

  uint8_t tw = (double)w / distance;
  uint8_t th = (double)h / distance;
  uint8_t pixel_size = etl::max(1.0, 1.0 / distance);

  // Don't draw the whole sprite if the anchor is hidden by z buffer
  // Not checked per pixel for performance reasons
  if (zbuffer[etl::min(etl::max((int)x, 0), ZBUFFER_SIZE - 1) / Z_RES_DIVIDER] <
      distance * DISTANCE_MULTIPLIER) {
    return;
  }

  for (uint8_t ty = 0; ty < th; ty += pixel_size) {
    for (uint8_t tx = 0; tx < tw; tx += pixel_size) {
      uint8_t sprite_x = tx * distance;
      uint8_t sprite_y = ty * distance;

      auto byte_offset = sprite_y * w + sprite_x;
      auto pixel = pgm_read_word(bitmap + byte_offset);

      if (pixel != sdl::kBlack) {
        for (uint8_t ox = 0; ox < pixel_size; ox++) {
          for (uint8_t oy = 0; oy < pixel_size; oy++) {
            sdl::Sprite()->drawPixel(x + tx + ox, y + ty + oy, pixel);
          }
        }
      }
    }
  }
}

// Draw a string
void drawText(int8_t x, int8_t y, const char *txt, sdl::Color color) {
  sdl::Sprite()->setCursor(x, y);
  sdl::Sprite()->setTextColor(color);
  sdl::Sprite()->print(txt);
}

void drawBitmap(int8_t x, int8_t y, const uint8_t *data, int16_t w, int16_t h,
                int16_t color) {

  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7) {
        byte <<= 1;
      } else {
        byte = pgm_read_byte(&data[j * byteWidth + i / 8]);
      }

      if (byte & 0x80) {
        drawPixel(x + i, y, color);
      }
    }
  }
}
