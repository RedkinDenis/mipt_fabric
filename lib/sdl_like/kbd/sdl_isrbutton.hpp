/*
 * Button class for attaching to interupts
 *
 */

#pragma once
#include <Arduino.h>

namespace sdl {

constexpr long kDebounceDelay = 50;

IRAM_ATTR void ButtonHandler(void *arg);

template <int DigitalPin> class IsrButton {
public:
  IsrButton() : clicked_(false) {}
  void init() {
    pinMode(DigitalPin, INPUT_PULLUP);
    attachInterruptArg(DigitalPin, &ButtonHandler, this, FALLING);
  }

  void clicked() { clicked_ = true; }
  bool click() { return std::exchange(clicked_, false); }

private:
  volatile bool clicked_;
};

IRAM_ATTR inline void ButtonHandler(void *arg) {
  static long last_access_time = 0;

  auto button = static_cast<IsrButton<D6> *>(arg);
  long access_time = millis();
  // Serial.println("clicked\n");
  button->clicked();

  if (access_time - last_access_time > kDebounceDelay) {
    button->clicked();
    last_access_time = access_time;
  }
}

}; // namespace sdl
