#pragma once
#include <etl/string.h>
#include <sdl_types.hpp>

namespace gf {

class Button {
public:
  Button(const etl::string<10> &label, sdl::u8 row, sdl::u8 col);
  void draw();

  void activate();
  void deactivate();

private:
  etl::string<10> label_;
  sdl::u8 row_;
  sdl::u8 col_;
  bool active_;
};
} // namespace gf