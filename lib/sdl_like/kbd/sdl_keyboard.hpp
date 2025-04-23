#pragma once

#ifdef SIM
#include "sdl_key.hpp"
#include <SFML/Window.hpp>
#else
#include "sdl_button.hpp"
#include "sdl_buttonpad.hpp"
#endif

namespace sdl {
template <int AnalogPin, int Digital1, int Digital2> class Keyboard {
public:
  void init() {
#ifndef SIM
    menu_button_.init();
    back_button_.init();
    arrow_buttons_.init();
#endif
  }

  Key getKey() {
#ifdef SIM
    return _sim_getKey();
#else
    return _getKey();
#endif
  }

private:
#ifdef SIM
  Key _sim_getKey() {
    extern sf::Event key_buff;

    switch (key_buff.key.code) {

    case sf::Keyboard::Up:
    case sf::Keyboard::W:
      return sdl::KeyUp;

    case sf::Keyboard::Down:
    case sf::Keyboard::S:
      return sdl::KeyDown;

    case sf::Keyboard::Right:
    case sf::Keyboard::D:
      return sdl::KeyRight;

    case sf::Keyboard::Left:
    case sf::Keyboard::A:
      return sdl::KeyLeft;

    case sf::Keyboard::Space:
      return sdl::KeyBack;
    case sf::Keyboard::Enter:
      return sdl::KeySelect;

    default:
      return sdl::KeyNone;
    }
  }

#else

  Key _getKey() {
    if (menu_button_.click()) {
      return sdl::KeySelect;
    }
    if (back_button_.click()) {
      return sdl::KeyBack;
    }
    return arrow_buttons_.getKey();
  }

  sdl::experimental::SerialButtonPad<AnalogPin> arrow_buttons_;
  sdl::Button<Digital1> menu_button_;
  sdl::Button<Digital2, true> back_button_;

#endif // SIM
};
} // namespace sdl