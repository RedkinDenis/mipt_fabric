#include <Arduino.h>
#include <app.hpp>

#include <doom.hpp>
#include <menu.hpp>
#include <pong.hpp>
#include <tetris.hpp>

#include <kbd/sdl_keyboard.hpp>

#ifndef SIM
#include <TFT_eSPI.h>
#else
#include <sdl/sim_display.hpp>
#endif // SIM
#include <etl/to_string.h>

sdl::Keyboard<A0, D8, D4> keyboard;

App gApps[] = {[kPong] = {PongStep},
               [kTetris] = {TetrisStep},
               [kMenu] = {MenuStep},
               [kDoom] = {DoomStep}};

constexpr int kFrameTick = 50; // ms

void setup() {
#ifndef SIM
  Serial.begin(9600);
#endif // SIM

  keyboard.init();
  PongInit();
  DoomInit();
  // sdl::AllocSprite(sdl::kWidth, 110);
}

void loop() {
  static uint64_t next_game_tick = 0;
  static enum AppsList sCurrentApp = kMenu;

  static sdl::Key key_buffer = sdl::KeyNone;

  auto key = keyboard.getKey();
  if (key != sdl::KeyNone) {
    key_buffer = key;

#ifndef SIM
    if (key_buffer.pressed()) {
      Serial.printf("pressed %d\n", key_buffer.code());
    } else {
      Serial.printf("clicked %d\n", key_buffer.code());
    }
#endif // SIM
  }

  if (millis() - next_game_tick < kFrameTick) {
    return;
  }

  next_game_tick = millis();

  sCurrentApp = gApps[sCurrentApp].main_(key_buffer);
  key_buffer = sdl::KeyNone;
}

#ifdef SIM
namespace sdl {
sf::Event key_buff;
}

int main() {
  setup();

  long last_time = 0;
  auto &&display = sim::Display::GetOrInit();

  while (display->isOpen()) {
    sf::Event event;

    while (true) {
      display->pollEvent(event);
      if (event.type == sf::Event::Closed ||
          event.type == sf::Event::KeyPressed &&
              event.key.code == sf::Keyboard::Escape) {
        display->close();
        break;
      }

      if (event.type == sf::Event::KeyPressed)
        sdl::key_buff = event;

      loop();
      sdl::key_buff.key.code = sf::Keyboard::Unknown;

      if (millis() - last_time > 3) {
        display->flush();
        last_time = millis();
      }
    }
  }

  return 0;
}
#endif // SIM