#pragma once

#include <Arduino.h>

#include <sdl_types.hpp>

namespace sdl {

template <int DigitalPin, bool Invert = false> class Button {
public:
  Button() : time_(0), clicked_(false) {}

  void init() { pinMode(DigitalPin, INPUT); }

  bool click() {
    bool buttonState = false;
    if constexpr (Invert) {
      buttonState = !digitalRead(DigitalPin);
    } else {
      buttonState = digitalRead(DigitalPin);
    }
    if (buttonState && !clicked_ && millis() - time_ >= 50) {
      clicked_ = true;
      time_ = millis();
      return true;
    }
    if (buttonState && clicked_ && millis() - time_ >= 200) {
      time_ = millis();
      return true;
    }
    if (!buttonState && clicked_) {
      clicked_ = false;
      time_ = millis();
    }
    return false;
  }

private:
  u32 time_;
  bool clicked_;
};

constexpr int kClickDelay = 120;

template <int DigitalPin> class HardwareDebounceButton {
public:
  HardwareDebounceButton() : time_(0), clicked_(false) {}

  void init() { pinMode(DigitalPin, INPUT); }

  bool click() {
    auto current = digitalRead(DigitalPin);
    if (!clicked_ && current) {
      clicked_ = true;
      return clicked_;
    }
    if (clicked_ && !current) {
      clicked_ = false;
    }
    return false;
  }

private:
  u32 time_;
  bool clicked_;
};
} // namespace sdl