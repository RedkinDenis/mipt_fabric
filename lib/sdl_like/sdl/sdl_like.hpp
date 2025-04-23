#pragma once

#ifndef SIM
#include <SPI.h>
#include <TFT_eSPI.h>
#else
#include "sim_display.hpp"
#endif

#include <etl/string.h>
#include <sdl_types.hpp>

namespace sdl {
//  RGB(5, 6, 5)
// P.s big endian endian

enum Color : u16 {
  kBlack = 0x0000,
  kGreen = 0x07E0,
  kRed = 0xF800,
  kWhite = 0xFFFF,
  kBlue = 0x001F,
  kOrange = 0xFEE4,
  kLightGreen = 0x3CAD,
  kTetrisGrey = 0x9D76,
  kDarkGrey = 0x4A49,
  kDoomOrange = 0xFCC9,
};

struct vec {
  vec(u8 x, u8 y) : x(x), y(y) {}
  vec() = default;

  u8 x{};
  u8 y{};
};

u16 Colorize(u8 r, u8 g, u8 b);

void DrawLine(u8 x0, u8 y0, u8 x1, u8 y1, u16 color);

// fast draw horizintal line
void FastHL(u8 x, u8 y, u8 width, u16 color);
// fast draw vertical line
void FastVL(u8 x, u8 y, u8 height, u16 color);

void Fill(u16 color);

void DrawCircle(u8 x, u8 y, u8 r, u16 color, bool filled = true);
void DrawRect(u8 x, u8 y, u8 width, u8 height, u16 color, bool filled = true);

enum class Orientation : u8 {
  kHorizontal,
  kVertical,
};
void SetOrientation(Orientation o);

// fmt-like print string
void Print(const etl::string<256> &format, u16 color = kWhite, vec offset = {});
void Print(const etl::string<256> &format, u16 color, u16 bgcolor,
           vec offset = {});
void PrintLn(const etl::string<256> &format, u16 color = kWhite,
             vec offset = {});

void drawImage(u8 x, u8 y, u8 w, u8 h, const u16 *image);

// sprite support
#ifndef SIM
using SpriteT = TFT_eSprite;
#else
using SpriteT = sim::Display;
#endif // SIM

void AllocSprite(u8 w, u8 h, u8 bpp);
void DeallocSprite();

void PushSprite(u8 x, u8 y);
SpriteT *Sprite();

}; // namespace sdl