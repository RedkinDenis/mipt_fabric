#pragma once

#ifndef SIM
#include_next <Arduino.h>
#else

#include <chrono>

inline long millis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

#define A0 0
#define D8 0
#define D6 0
#define D4 0

#define PROGMEM

#define pgm_read_byte(addr) *((uint8_t *)addr)
#define pgm_read_word(addr) *((uint16_t *)addr)

#define PI 3.1415

#endif // SIM