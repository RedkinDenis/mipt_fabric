#pragma once

#include <kbd/sdl_key.hpp>

enum AppsList { kPong, kTetris, kMenu, kDoom };

class App {
public:
  using loop = AppsList (*)(sdl::Key);
  loop main_;
  // mb add init func
};

inline AppsList GetPrevApp() { return kMenu; }