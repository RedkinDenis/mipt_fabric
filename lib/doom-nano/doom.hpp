#pragma once
#include <sdl/sdl_like.hpp>

#include <app.hpp>

void DoomInit();
// return next app to schedule
AppsList DoomStep(sdl::Key key);