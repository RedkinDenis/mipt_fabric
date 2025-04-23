#include <button/button.hpp>
#include <logo/menu.hpp>
#include <menu.hpp>

static struct MenuState {
  static constexpr int kButtons = 4;

private:
  std::array<gf::Button, kButtons> buttons = {gf::Button{"pong", 6, 7},
                                              {"tetris", 8, 7},
                                              {"snake", 6, 17},
                                              {"doom", 8, 17}};
  std::array<AppsList, kButtons> actions{kPong, kTetris, kPong, kDoom};
  int active = 0;

public:
  void draw() {
    // sdl::Fill(sdl::kGreen);
    sdl::DrawRect(5, 5, sdl::kWidth - 5, sdl::kHeight - 5, sdl::kBlack, false);
    // sdl::PrintLn("JOS BOY", sdl::kWhite, {3, 12});
    sdl::drawImage(20, 20, gf::kLogoWidth, gf::kLogoHeight, gf::kLogo);
    for (auto &button : buttons) {
      button.draw();
    }
  }
  void nextActive() {
    buttons[active].deactivate();
    active = (active + 1) % kButtons;
    buttons[active].activate();
  }
  void prevActive() {
    buttons[active].deactivate();
    active = (kButtons + active - 1) % kButtons;
    buttons[active].activate();
  }
  AppsList currentActiveAppButton() { return actions[active]; }

} gState;

AppsList MenuStep(sdl::Key key) {
  static bool need_update = true;

  if (key != sdl::KeyNone) {
    if (key == sdl::KeyUp) {
      gState.prevActive();
    } else if (key == sdl::KeyDown) {
      gState.nextActive();
    } else if (key == sdl::KeySelect) {
      sdl::Fill(sdl::kBlack);
      need_update = true;
      return gState.currentActiveAppButton();
    }
    need_update = true;
  }
  if (need_update) {
    gState.draw();
    need_update = false;
  }

  return kMenu;
}