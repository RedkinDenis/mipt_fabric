#pragma once

enum class State {
  kMenu,
  kRun,
  kExit,
  kPause,
};

class Grid;

class Piece {
public:
  Piece() { next(); }
  Piece(short rotation, int type);

  void rotate(const Grid &grid);
  void next();

  void draw();
  void drawNext();

  auto getFigure() const;

private:
  bool canRotate(const Grid &grid, short rotation);

public:
  short x;
  short y;

  sdl::Color color_;

private:
  int rotation_;
  int type_;
  int next_type;
};

class Grid {
public:
  size_t clearLines();

  void draw();

  void fillPiece(const Piece &piece);

public:
  sdl::Color buf[cfg::grid::kWidth][cfg::grid::kHeight];

private:
  void clearLine(short line);
};

bool nextHorizontalCollision(const Piece &piece, const Grid &grid, int amount);
bool nextCollision(const Piece &piece, const Grid &grid);
