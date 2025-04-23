#include <etl/to_string.h>
#include <kbd/sdl_key.hpp>
#include <sdl/sdl_like.hpp>

#include <Arduino.h>
#include <config.hpp>
#include <iostream>
#include <logo/tetris.hpp>
#include <tetris.hpp>
#include <tetris_impl.hpp>

int score = 0;
State gState = State::kMenu;

Piece piece;
Grid grid;

static void DrawLayout() {
  // draw box area
  sdl::FastHL(cfg::kMarginLeft - 1, cfg::grid::kPixHeight + cfg::kMarginTop,
              cfg::grid::kPixWidth + 1, sdl::kWhite);

  sdl::FastVL(cfg::kMarginLeft - 1, cfg::kMarginTop, cfg::grid::kPixHeight,
              sdl::kWhite);

  sdl::FastVL(cfg::kMarginLeft + cfg::grid::kPixWidth, cfg::kMarginTop,
              cfg::grid::kPixHeight, sdl::kWhite);

  sdl::PrintLn("Tetris", sdl::kWhite, {2, 3});
}

static void DrawScore() {
  etl::string<10> score_str;
  etl::to_string(score, score_str, true);
  sdl::Print(score_str, sdl::kWhite, sdl::kBlack, {5, 8});
}

static void DrawGrid() {
  // sdl::DrawRect(cfg::kMarginLeft, cfg::kMarginTop, cfg::grid::kPixWidth,
  //               cfg::grid::kPixHeight, sdl::kBlack);
  grid.draw();
}

void refresh() {
  // TODO(egor): optimize refresh
  DrawGrid();
  DrawScore();
  DrawLayout();
  piece.draw();
  piece.drawNext();
}

static AppsList TetrisPlay(sdl::Key key) {
  long delay = cfg::kDelay;
  static int step = 0;

  switch (key.code()) {
  case sdl::KeyLeft:
    if (!nextHorizontalCollision(piece, grid, -1)) {
      piece.x--;
      refresh();
    }
    break;
  case sdl::KeyRight:
    if (!nextHorizontalCollision(piece, grid, 1)) {
      piece.x++;
      refresh();
    }
    break;
  case sdl::KeyDown:
    delay = cfg::kFastDelay;
    break;
  case sdl::KeySelect:
    piece.rotate(grid);
    refresh();
    break;
  case sdl::KeyBack:
    gState = State::kPause;
    break;
  default:
    break;
  }

  if (++step % delay != 0) {
    return kTetris;
  }

  auto cleared = grid.clearLines();
  score += cleared * 10;

  refresh();

  if (nextCollision(piece, grid)) {
    grid.fillPiece(piece);
    if (piece.y == 0) {
      // game over
      gState = State::kMenu;
      piece.next();
      grid = {};
    }
    piece.next();
  } else
    piece.y++;

  return kTetris;
}

static void DrawButtons(const char *first, const char *second) {
  sdl::Print(first, sdl::kDarkGrey, sdl::kTetrisGrey, {8, 10});
  sdl::Print(second, sdl::kDarkGrey, sdl::kTetrisGrey, {10, 10});
}

static AppsList TetrisMenu(sdl::Key key) {
  static bool init = false;
  static bool active = 0;

  if (!init) {
    sdl::Fill(sdl::Color::kTetrisGrey);
    sdl::drawImage((sdl::kWidth - gf::kTetrisLogoWidth) / 2,
                   gf::kTetrisLogoHeight / 4, gf::kTetrisLogoWidth,
                   gf::kTetrisLogoHeight, gf::kTetrisLogo);

    DrawButtons("  Start", "  Exit");
    init = true;
  }

  switch (key.code()) {
  case sdl::KeyUp:
    DrawButtons("> Start", "  Exit");
    active = 0;
    break;
  case sdl::KeyDown:
    DrawButtons("  Start", "> Exit");
    active = 1;
    break;
  case sdl::KeySelect:
    sdl::Fill(sdl::kBlack);
    gState = active ? State::kExit : State::kRun;
    init = false;
    break;
  default:
    break;
  }

  return kTetris;
}

AppsList TetrisStep(sdl::Key key) {
  switch (gState) {
  case State::kMenu:
  case State::kPause:
    return TetrisMenu(key);
  case State::kRun:
    return TetrisPlay(key);
  case State::kExit:
    gState = State::kMenu;
    break;
  default:
    break;
  }

  return GetPrevApp();
}