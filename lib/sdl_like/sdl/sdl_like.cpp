#ifndef SIM
#include <SPI.h>
#include <TFT_eSPI.h>
#else
#include "sim_display.hpp"
#endif

#include <etl/format_spec.h>
#include <etl/string.h>
#include <etl/string_stream.h>

#include <sdl/sdl_like.hpp>

namespace sdl {
#ifndef SIM
using Display = TFT_eSPI;
#else
using Display = sim::Display;
#endif // SIM

Display *GetOrInit() {
#ifndef SIM
  static Display local_tft = [] {
    Display tft = Display();
    tft.init();
    return tft;
  }();

  return &local_tft;
#else
  return Display::GetOrInit();
#endif // SIM
}

#ifdef SIM
static SpriteT *sSprite = nullptr;

void PushSprite(u8 x, u8 y) {}
void AllocSprite(u8 w, u8 h, u8 bpp) { sSprite = GetOrInit(); }

SpriteT *Sprite() { return sSprite; }

void DeallocSprite() {}
#else
static SpriteT sSprite(GetOrInit());

void PushSprite(u8 x, u8 y) { sSprite.pushSprite(x, y); }

void AllocSprite(u8 w, u8 h, u8 bpp) {
  sSprite.setColorDepth(bpp);
  sSprite.createSprite(w, h);
}

SpriteT *Sprite() { return &sSprite; }

void DeallocSprite() { sSprite.deleteSprite(); }
#endif

void DrawLine(u8 x0, u8 y0, u8 x1, u8 y1, u16 color) {
  auto screen = GetOrInit();
  screen->drawLine(x0, y0, x1, y1, color);
}

void FastHL(u8 x, u8 y, u8 width, u16 color) {
  auto screen = GetOrInit();
  screen->drawFastHLine(x, y, width, color);
}

void FastVL(u8 x, u8 y, u8 height, u16 color) {
  auto screen = GetOrInit();
  screen->drawFastVLine(x, y, height, color);
}

void DrawCircle(u8 x, u8 y, u8 r, u16 color, bool fill) {
  auto screen = GetOrInit();
  if (fill) {
    screen->fillCircle(x, y, r, color);
  } else {
    screen->drawCircle(x, y, r, color);
  }
}

void DrawRect(u8 x, u8 y, u8 width, u8 height, u16 color, bool fill) {
  auto screen = GetOrInit();
  if (fill) {
    screen->fillRect(x, y, width, height, color);
  } else {
    screen->drawRect(x, y, width, height, color);
  }
}

void Fill(u16 color) {
  auto screen = GetOrInit();
  screen->fillScreen(color);
}

void Print(const etl::string<256> &format, u16 color, vec offset) {
  auto screen = GetOrInit();

  screen->setCursor(offset.y * kFontWidth, offset.x * kFontHeight);

  screen->setTextColor(color);
  screen->print(format.c_str());
}

void Print(const etl::string<256> &format, u16 color, u16 bgcolor, vec offset) {
  auto screen = GetOrInit();

  screen->setCursor(offset.y * kFontWidth, offset.x * kFontHeight);

  screen->setTextColor(color, bgcolor, true);
  screen->print(format.c_str());
}

void PrintLn(const etl::string<256> &format, u16 color, vec offset) {
  auto screen = GetOrInit();

  screen->setCursor(offset.y * kFontWidth, offset.x * kFontHeight);
  screen->setTextColor(color);
  screen->println(format.c_str());
}


void drawImage(u8 x, u8 y, u8 w, u8 h, const u16* image) {
  auto screen = GetOrInit();
  screen->pushImage(x, y, w, h, image);
}

u16 Colorize(u8 r, u8 g, u8 b) {
  if (r < 0 || 255 < r || g < 0 || 255 < g || b < 0 || b > 255)
    return -1;

  u16 red = r * 31 / 255;
  u16 green = g * 63 / 255;
  u16 blue = b * 31 / 255;

  green = green << 5;
  red = red << 11;

  return red | green | blue;
}

void SetOrientation(Orientation o) {
  auto screen = GetOrInit();

  switch (o) {
  case Orientation::kHorizontal:
    screen->setRotation(3);
    break;
  case Orientation::kVertical:
    screen->setRotation(2);
    break;
  default:
    break;
  }
}

} // namespace sdl