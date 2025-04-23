#pragma once
#include <sdl/sdl_like.hpp>

enum class State {
    MENU,
    GAME_OVER,
    GAME_RUN,
    PAUSE,
    EXIT,
};


constexpr int kMaxScore = 2;
constexpr int kPaddleWidth = 5;
constexpr int kPaddleHeight = 30;
constexpr int kPaddlePadding = 5;
constexpr int kBallSize = 3;
constexpr int kPlayerPaddleSpeed = 3;
constexpr int kAIPaddleSpeed = 15;
constexpr int kFrameWidth = 2;

struct game_data_t;
struct rectangle_t;

constexpr sdl::u16 kHeight = 128;
constexpr sdl::u16 kWidth = 160;

struct rectangle_t {
  int x_, y_;
  int w_, h_;
  sdl::u32 color_;
  
  rectangle_t() : rectangle_t{0,0,0,0, sdl::kWhite} {}

  rectangle_t(int x, int y, int w, int h, sdl::u32 color) :
    x_{x}, y_{y}, w_{w}, h_{h}, color_{color} {}

  virtual void draw() {};
  virtual void move(game_data_t*) {};
};

struct paddle_t : rectangle_t {
  int v_y;
  
  paddle_t(int x, int y, uint32_t color) : rectangle_t(x, y, kPaddleWidth, kPaddleHeight, color),
                                           v_y{0}
  {}
  
  paddle_t() : paddle_t(0, 0, sdl::kWhite) {}

  virtual void draw() override;
  virtual void move(game_data_t*) override;
};

struct ai_paddle_t : paddle_t {
  ai_paddle_t(int x, int y, uint32_t color) : paddle_t(x, y,color)
  {}

  ai_paddle_t() : ai_paddle_t(0, 0, sdl::kWhite) {}

  virtual void move(game_data_t*) override;
};

struct ball_t : rectangle_t {
  int v_x, v_y;

  ball_t(int x, int y) : rectangle_t(x, y, kBallSize, kBallSize, sdl::kWhite), 
                        v_x{1}, v_y{1}
  {}
  ball_t() : ball_t(0, 0) {}

  virtual void draw() override;
  virtual void move(game_data_t*) override;
};

struct game_data_t {
  ball_t ball; 
  ai_paddle_t ai_paddle;
  paddle_t paddle;
  rectangle_t net;
  
  void reset_figures() {
    ball = ball_t {kWidth / 2, kHeight / 2};
    ai_paddle = ai_paddle_t {kPaddlePadding, kHeight / 2 - kPaddleHeight, sdl::kGreen};
    paddle = paddle_t {kWidth - kPaddlePadding - kPaddleWidth, kHeight / 2 - kPaddleHeight, sdl::kWhite};
    net = rectangle_t {kWidth / 2, 20, 5, 25, sdl::kWhite}; 
  }
  
  void reset_score() {
    ai_score = 0;
    user_score = 0;
  }
  
  rectangle_t gates[2];

  rectangle_t *const objects[6] = {&ai_paddle, &paddle, &ball, &net, &gates[0], &gates[1]};
  
  const int ndrawable {6};
  const int nmovable {3};

  int ai_score;
  int user_score;

  State state {State::MENU};
};
