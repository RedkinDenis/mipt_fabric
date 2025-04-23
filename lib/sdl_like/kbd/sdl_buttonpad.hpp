#pragma once
#include <Arduino.h>

#include "sdl_key.hpp"
#include "sdl_types.hpp"

namespace sdl {

constexpr int kDelta = 50;
constexpr int kDelay = 70;
constexpr int kPressDelay = 140;

// TODO(egor): refactor it
constexpr int kButton1Value = 1024;
constexpr int kButton2Value = 730;
constexpr int kButton3Value = 450;
constexpr int kButton4Value = 330;

inline KeyCode VoltageToKeyCode(int voltage) {
  if (std::abs(kButton4Value - voltage) < kDelta) {
    return sdl::KeyRight;
  }
  if (std::abs(kButton3Value - voltage) < kDelta) {
    return sdl::KeyLeft;
  }
  if (std::abs(kButton2Value - voltage) < kDelta) {
    return sdl::KeyDown;
  }
  if (std::abs(kButton1Value - voltage) < kDelta) {
    return sdl::KeyUp;
  }
  return sdl::KeyNone;
}

template <int AnalogPin> class ParallelButtonPad {
public:
  ParallelButtonPad() : old_key_(0), last_change_(0), pressed_(false) {}

  void init() { pinMode(AnalogPin, INPUT); }

  Key getKey() {
    int actual_key = analogRead(AnalogPin); // Получаем актуальное состояние

    auto current_time = millis();
    if (current_time - last_change_ > kDelay) {
      if (std::abs(actual_key - old_key_) >= kDelta) {

        // Пришло новое значение, и с последнего
        // изменения прошло достаточно времени
        pressed_ = false;
        old_key_ = actual_key; // Запоминаем новое значение
        last_change_ = millis(); // Обнуляем таймер

        return VoltageToKeyCode(actual_key);
      } else if (pressed_) {
        return {VoltageToKeyCode(actual_key), true};
      }
    }

    if (current_time - last_change_ > kPressDelay &&
        std::abs(actual_key - old_key_) < kDelta) {
      pressed_ = true;
      // пришло старое значение => кнопка зажата
      last_change_ = millis(); // Обнуляем таймер

      return {VoltageToKeyCode(actual_key), true};
    }
    return sdl::KeyNone;
  }

private:
  int old_key_;
  long last_change_;
  bool pressed_;
};

namespace experimental {

constexpr int kDelay = 140;
constexpr int kPressDelay = 200;

inline sdl::KeyCode VoltageToKeyCode(int voltage) {
  auto new_key = static_cast<sdl::KeyCode>((voltage + 100) >> 8);
  return new_key;
}

template <int AnalogPin> class SerialButtonPad {
public:
  SerialButtonPad()
      : old_key_(sdl::KeyNone), last_change_(0), pressed_(false) {}

  void init() { pinMode(AnalogPin, INPUT); }

  sdl::Key getKey() {
    auto actual_key = VoltageToKeyCode(
        analogRead(AnalogPin)); // Получаем актуальное состояние
    // if (old_key_ > actual_key) {
    //   old_key_ = sdl::KeyNone;
    //   return old_key_;
    // }

    auto current_time = millis();
    if (current_time - last_change_ > kDelay) {
      if (actual_key != old_key_) {

        // Пришло новое значение, и с последнего
        // изменения прошло достаточно времени
        pressed_ = false;
        old_key_ = actual_key; // Запоминаем новое значение
        last_change_ = millis(); // Обнуляем таймер

        return actual_key;
      } else if (pressed_) {
        return {actual_key, true};
      }
    }

    // press detection
    if (current_time - last_change_ > kPressDelay && actual_key == old_key_) {
      pressed_ = true;
      // пришло старое значение => кнопка зажата
      last_change_ = millis(); // Обнуляем таймер

      return {actual_key, true};
    }
    return sdl::KeyNone;
  }

private:
  sdl::KeyCode old_key_;
  long last_change_;
  bool pressed_;
};

} // namespace experimental

} // namespace sdl