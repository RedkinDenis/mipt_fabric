#include <button/button.hpp>

#include <sdl/sdl_like.hpp>
#ifndef PLAIN_BUTTON
#include <button/sprite.hpp>
#endif

namespace gf {

Button::Button(const etl::string<10> &label, sdl::u8 row, sdl::u8 col)
    : label_(label), row_(row), col_(col), active_(false) {}

void Button::draw() {
#ifdef PLAIN_BUTTON
  if (active_) {
    sdl::Print(label_, sdl::kBlack, sdl::kWhite, {row_, col_});

  } else {
    sdl::Print(label_, sdl::kWhite, sdl::kBlack, {row_, col_});
  }
#else
  if (active_) {
    sdl::drawImage(col_ * sdl::kFontWidth - 4, row_ * sdl::kFontHeight - 4, 48,
                   16, gf::kPressed);
    sdl ::Print(label_, sdl::kWhite, {row_, col_});
  } else {
    sdl::drawImage(col_ * sdl::kFontWidth - 4, row_ * sdl::kFontHeight - 4, 48,
                   16, gf::kReleased);
    sdl::Print(label_, sdl::kBlack, {row_, col_});
  }
#endif
}

void Button::activate() { active_ = true; }

void Button::deactivate() { active_ = false; }

}; // namespace gf