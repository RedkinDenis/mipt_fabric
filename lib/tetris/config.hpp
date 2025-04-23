#pragma once

// config
namespace cfg {
constexpr int kMarginTop = 10;
constexpr int kMarginLeft = 65;
constexpr int kNextPieceMarginLeft = 5;
constexpr int kNextPieceMarginTop = 45;
constexpr int kNextPieceSize = 4;

constexpr short kSize = 5;

enum Type {
  kSLeft,
  kSRight,
  kLLeft,
  kSquare,
  kT,
  kL,
};

constexpr short kTypes = 6;

namespace grid {
// constexpr int kSize = 10;
constexpr int kWidth = 15;
constexpr int kHeight = 18;

constexpr int kPixWidth = kWidth * (cfg::kSize + 1);
constexpr int kPixHeight = kHeight * (cfg::kSize + 1);
} // namespace grid

constexpr long kDelay = 8;
constexpr long kFastDelay = 1;
} // namespace cfg