#include <config.hpp>
#include <cstring>
#include <sdl/sdl_like.hpp>
#include <tetris_impl.hpp>

constexpr size_t kPieceSize = 2 * 4 * sizeof(char);
template <size_t MaxRotation> using Figure = char[MaxRotation][2][4];

constexpr Figure<2> kFigureZ = {{{0, 0, 1, 1}, {0, 1, 1, 2}},
                                {{0, 1, 1, 2}, {1, 1, 0, 0}}};

constexpr Figure<2> kFigureS = {{{1, 1, 0, 0}, {0, 1, 1, 2}},
                                {{0, 1, 1, 2}, {0, 0, 1, 1}}};

constexpr Figure<4> kFigureL = {{{0, 0, 0, 1}, {0, 1, 2, 2}},
                                {{0, 1, 2, 2}, {1, 1, 1, 0}},
                                {{0, 1, 1, 1}, {0, 0, 1, 2}},
                                {{0, 0, 1, 2}, {1, 0, 0, 0}}};

constexpr Figure<1> kFigureO = {{{0, 1, 0, 1}, {0, 0, 1, 1}}};

constexpr Figure<4> kFigureT = {{{0, 0, 1, 0}, {0, 1, 1, 2}},
                                {{0, 1, 1, 2}, {1, 0, 1, 1}},
                                {{1, 0, 1, 1}, {0, 1, 1, 2}},
                                {{0, 1, 1, 2}, {0, 0, 1, 0}}};

constexpr Figure<2> kFigureI = {{{0, 0, 0, 0}, {0, 1, 2, 3}},
                                {{0, 1, 2, 3}, {0, 0, 0, 0}}};

constexpr size_t kFiguresCount = 6;

constexpr std::array<const char (*)[2][4], kFiguresCount> figures = {
    kFigureZ, kFigureS, kFigureL, kFigureO, kFigureT, kFigureI};

constexpr std::array<size_t, kFiguresCount> kMaxRotation = {2, 2, 4, 1, 4, 2};

constexpr std::array<sdl::Color, 4> colors = {sdl::kRed, sdl::kTetrisGrey,
                                              sdl::kLightGreen, sdl::kOrange};

auto Piece::getFigure() const { return figures[type_][rotation_]; }

size_t Grid::clearLines() {
  size_t cleared = 0;
  bool full = false;
  for (short y = cfg::grid::kHeight - 1; y >= 0; --y) {
    full = true;
    for (short x = 0; x < cfg::grid::kWidth; ++x) {
      full = full && buf[x][y];
    }
    if (full) {
      clearLine(y);
      cleared += 1;
      y += 1;
    }
  }
  return cleared;
}

// функция очистки завершенных строк
void Grid::clearLine(short line) {
  for (short y = line; y >= 0; y--) {
    for (short x = 0; x < cfg::grid::kWidth; x++) {
      buf[x][y] = buf[x][y - 1];
    }
  }
  for (short x = 0; x < cfg::grid::kWidth; x++) {
    buf[x][0] = sdl::Color::kBlack;
  }
}

void Grid::draw() {
  for (short x = 0; x < cfg::grid::kWidth; x++)
    for (short y = 0; y < cfg::grid::kHeight; y++)
    //   if (buf[x][y] != sdl::Color::kBlack)
    {
      auto grid_x = cfg::kMarginLeft + (cfg::kSize + 1) * x;
      auto grid_y = cfg::kMarginTop + (cfg::kSize + 1) * y;

      sdl::DrawRect(grid_x, grid_y, cfg::kSize, cfg::kSize, buf[x][y]);
    }
}

void Grid::fillPiece(const Piece &piece) {
  for (short i = 0; i < 4; i++)
    buf[piece.x + piece.getFigure()[0][i]][piece.y + piece.getFigure()[1][i]] =
        piece.color_;
}

// проверка столкновения в горизонтальном направлении
bool nextHorizontalCollision(const Piece &piece, const Grid &grid, int amount) {
  for (short i = 0; i < 4; i++) {
    short new_x = piece.x + piece.getFigure()[0][i] + amount;
    short new_y = piece.y + piece.getFigure()[1][i];

    if (new_x >= cfg::grid::kWidth || new_x < 0 || grid.buf[new_x][new_y])
      return true;
  }
  return false;
}

// проверка столкновения в вертикальном направлении
bool nextCollision(const Piece &piece, const Grid &grid) {
  for (short i = 0; i < 4; i++) {
    short y = piece.y + piece.getFigure()[1][i] + 1;
    short x = piece.x + piece.getFigure()[0][i];
    if (y >= cfg::grid::kHeight || grid.buf[x][y])
      return true;
  }
  return false;
}

Piece::Piece(short rotation, int type) : rotation_(rotation), type_(type) {}

void Piece::rotate(const Grid &grid) {
  auto next_rotation = (rotation_ + 1) % kMaxRotation[type_];

  if (canRotate(grid, next_rotation)) {
    rotation_ = next_rotation;
  }
}

bool Piece::canRotate(const Grid &grid, short rotation) {
  Piece local(rotation, type_);
  local.x = x;
  local.y = y;
  return !nextHorizontalCollision(local, grid, 0);
}

void Piece::next() {
  type_ = next_type;
  color_ = colors[std::rand() % colors.size()];
  next_type = std::rand() % cfg::kTypes;
  x = std::rand() % (cfg::grid::kWidth - 1);
  y = 0;
  rotation_ = 0;
}

void Piece::draw() {
  for (short i = 0; i < 4; i++) {
    sdl::DrawRect(cfg::kMarginLeft + (cfg::kSize + 1) * (x + getFigure()[0][i]),
                  cfg::kMarginTop + (cfg::kSize + 1) * (y + getFigure()[1][i]),
                  cfg::kSize, cfg::kSize, color_);
  }
}

void Piece::drawNext() {
  Piece next_piece(0, next_type);

  sdl::DrawRect(cfg::kNextPieceMarginLeft, cfg::kNextPieceMarginTop,
                (cfg::kNextPieceSize + 1) * 4, (cfg::kNextPieceSize + 1) * 5,
                sdl::kWhite);

  for (short i = 0; i < 4; i++) {
    sdl::DrawRect(
        cfg::kNextPieceMarginLeft +
            (cfg::kNextPieceSize + 1) * (next_piece.getFigure()[0][i] + 1),
        cfg::kNextPieceMarginTop +
            (cfg::kNextPieceSize + 1) * (next_piece.getFigure()[1][i] + 1),
        cfg::kNextPieceSize, cfg::kNextPieceSize, sdl::kBlack);
  }
}
