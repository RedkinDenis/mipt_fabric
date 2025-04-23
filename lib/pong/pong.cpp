#include <etl/to_string.h>

#include "pong_details.hpp"
#include <logo/pong.hpp>
#include <pong.hpp>

game_data_t game_info;
int check_collision(ball_t *ball, paddle_t *paddle);

void ball_t::draw() { sdl::DrawCircle(x_, y_, kBallSize, color_); }

void ball_t::move(game_data_t *data) {
  color_ = sdl::kBlack;
  draw();
  color_ = sdl::kWhite;

  x_ += v_x;
  y_ += v_y;

  if (y_ < 0 || y_ > kHeight - 10) {
    v_y = -v_y;
  }
  for (int i = 0; i < 2; i++) {
    paddle_t *paddle = i == 1 ? &data->paddle : &data->ai_paddle;
    int collision = check_collision(this, paddle);
    if (collision) {
      // int y_offset = paddle->y_ - y_;

      if (v_x < 0) {
        v_x -= 1;
      } else {
        v_x += 1;
      }

      v_x = -v_x;

      if (x_ < w_) {
        x_ = w_;
      }
      // ball moving left
      else if (x_ > kWidth - w_) {
        x_ = kWidth - w_;
      }
    }
  }
}

void paddle_t::draw() {
  sdl::DrawRect(x_, y_ - 10, w_, h_ + 20, sdl::kBlack, true);
  sdl::DrawRect(x_, y_, w_, h_, color_, true);
}

void paddle_t::move(game_data_t *data) {
  y_ += v_y;
  if (y_ >= kHeight - h_) {
    y_ = kHeight - h_;
  } else if (y_ <= 5) {
    // TODO
    y_ = 5;
  }
}

void ai_paddle_t::move(game_data_t *data) {
  ball_t *ball = &data->ball;
  int center = y_ + kPaddleHeight / 2;
  int screen_center = kHeight / 2 - kPaddleHeight / 2;
  int ball_speed = ball->v_y > 0 ? ball->v_y : -ball->v_y;

  if (ball->v_x > 0) {
    // return to center position
    if (center < screen_center - kPaddleHeight / 2) {
      y_ += ball_speed;
    } else if (center > screen_center + kPaddleHeight / 2) {
      y_ -= ball_speed;
    }
  } else {
    // ball moving down
    if (ball->v_y > 0) {
      if (ball->y_ > center) {
        y_ += ball_speed;
      } else {
        y_ -= ball_speed;
      }
    }
    // ball moving up
    if (ball->v_y < 0) {
      if (ball->y_ < center) {
        y_ -= ball_speed;
      } else {
        y_ += ball_speed;
      }
    }
    // ball moving stright across
    if (ball->v_y == 0) {
      if (ball->y_ < center - kPaddleHeight / 2) {
        y_ -= v_y;
      } else if (ball->y_ > center + kPaddleHeight / 2) {
        y_ += v_y;
      }
    }
  }
  if (y_ <= 0) {
    y_ = 0;
  } else if (y_ >= kHeight - h_) {
    y_ = kHeight - h_;
  }
}

// if return value is 1 collision occured. if return is 0, no collision.
int check_collision(ball_t *ball, paddle_t *paddle) {
  if (ball->x_ > paddle->x_ + paddle->w_) {
    return 0;
  }
  if (ball->x_ + ball->w_ < paddle->x_) {
    return 0;
  }
  if (ball->y_ > paddle->y_ + paddle->h_) {
    return 0;
  }
  if (ball->y_ + ball->h_ < paddle->y_) {
    return 0;
  }
  return 1;
}

inline void gates_init(rectangle_t *gate) {

  for (int i = 0; i < 2; ++i) {
    gate[i].color_ = sdl::kWhite;
    gate[i].y_ = 0;
    gate[i].h_ = kHeight;
  }
  gate->x_ = 0;
  gate->w_ = kFrameWidth;

  gate[1].x_ = kWidth - kFrameWidth;
  gate[1].w_ = kFrameWidth;
}

enum State game_state(game_data_t *info) {
  // check over all games
  if (info->ai_score == kMaxScore) {
    sdl::PrintLn("You lose!", sdl::kGreen, {80 - 4 * 8, 60});
  } else if (info->user_score == kMaxScore) {
    sdl::PrintLn("You win!", sdl::kGreen, {80 - 4 * 8, 60});
  }
  if (info->user_score == kMaxScore || info->ai_score == kMaxScore) {
    return State::GAME_OVER;
  }

  return State::GAME_RUN;
}

void ResetFigures() {
  sdl::Fill(sdl::kBlack);

  game_info.reset_figures();

  game_info.paddle.v_y = kAIPaddleSpeed;
  game_info.paddle.v_y = kPlayerPaddleSpeed;

  gates_init(game_info.gates);
}

void FullReset() {
  ResetFigures();
  game_info.reset_score();
}

void PongInit() {
  sdl::SetOrientation(sdl::Orientation::kHorizontal);
  ResetFigures();
}

namespace {

void DrawButtons(const char *first, const char *second) {
  sdl::Print(first, sdl::kWhite, sdl::kLightGreen, {9, 11});
  sdl::Print(second, sdl::kWhite, sdl::kLightGreen, {11, 11});
}

void DrawLogo() {
  sdl::Fill(sdl::Color::kLightGreen);
  sdl::drawImage((160 - 106) / 2, 10, 106, 60, gf::kPongLogo);
}

void Pause(sdl::Key key) {
  static bool draw = false;
  static bool active = 0;
  if (!draw) {
    DrawLogo();
    DrawButtons("  Resume", "  Exit");
    draw = true;
  }

  switch (key.code()) {
  case sdl::KeyUp:
    DrawButtons("> Resume", "  Exit");
    active = 0;
    break;
  case sdl::KeyDown:
    DrawButtons("  Resume", "> Exit");
    active = 1;
    break;
  case sdl::KeySelect:
    sdl::Fill(sdl::kBlack);
    game_info.state = (active == 0) ? State::GAME_RUN : State::EXIT;
    draw = false;
  default:
    break;
  }
}

void Menu(sdl::Key key) {
  static bool draw = false;
  static bool active = 0;
  if (!draw) {
    DrawLogo();
    DrawButtons("  Start", "  Exit");
    draw = true;
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
    game_info.state = (active == 0) ? State::GAME_RUN : State::EXIT;
    FullReset();
    draw = false;
    break;
  default:
    break;
  }
}

void GameOver(sdl::Key key) {
  sdl::PrintLn("GAME OVER", sdl::kRed, {0, 10});
  sdl::PrintLn("press any key", sdl::kGreen, {1, 10});

  if (key != sdl::KeyNone) {
    FullReset();
    game_info.state = State::GAME_RUN;
  }

  return;
}

void Run(sdl::Key key) {
  if (game_state(&game_info) == State::GAME_OVER) {
    game_info.state = State::GAME_OVER;
    sdl::Fill(sdl::kBlack);
    return;
  }

  // check over one batch
  if (game_info.ball.x_ < 0) {
    game_info.user_score += 1;
    ResetFigures();
  }

  if (game_info.ball.x_ > kWidth - game_info.ball.w_) {
    game_info.ai_score += 1;
    ResetFigures();
  }

  etl::string<3> score;
  etl::to_string(game_info.ai_score, score);
  score += ":";
  etl::to_string(game_info.user_score, score, true);

  sdl::Print(score, sdl::kWhite, {0, 13});

  switch (key.code()) {
  case sdl::KeySelect:
    game_info.state = State::PAUSE;
    break;
  case sdl::KeyUp:
    game_info.paddle.v_y = -kPlayerPaddleSpeed;
    break;
  case sdl::KeyDown:
    game_info.paddle.v_y = kPlayerPaddleSpeed;
    break;
  case sdl::KeyNone:
    game_info.paddle.v_y = 0;
  default:; // idle
  }

  for (int i = 0; i < game_info.nmovable; ++i)
    game_info.objects[i]->move(&game_info);

  for (int i = 0; i < game_info.ndrawable; ++i)
    game_info.objects[i]->draw();
}

} // namespace

// return true -> if we should be reschduledl, false -> not
AppsList PongStep(sdl::Key key) {
  switch (game_info.state) {
  case State::MENU:
    Menu(key);
    break;
  case State::PAUSE:
    Pause(key);
    break;
  case State::GAME_RUN:
    Run(key);
    break;
  case State::GAME_OVER:
    GameOver(key);
    break;
  case State::EXIT:
    game_info.state = State::MENU;
    return GetPrevApp();
  default: // idle
    break;
  }

  return kPong;
}
