#pragma once
#include <etl/array.h>

#include <sdl_types.hpp>

namespace sdl {

enum KeyCode : u8 {
  KeyNone,

  KeyUp,
  KeyDown,
  KeyLeft,
  KeyRight,
  
  KeyBack,
  KeySelect,
};

class Key {
public:
  Key(KeyCode code) : code_(code), pressed_(false) {}
  Key(KeyCode code, bool pressed) : code_(code), pressed_(pressed) {}

  bool pressed() { return pressed_; }

  bool operator==(KeyCode code) { return code == code_; }
  bool operator!=(KeyCode code) { return !operator==(code); }

  KeyCode code() { return code_; }

private:
  KeyCode code_;
  bool pressed_;
};

} // namespace sdl
