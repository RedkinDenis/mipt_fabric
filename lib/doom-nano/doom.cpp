// #include <cmath.h>

#include "constants.h"
#include "display.h"
#include "entities.h"
#include "level.h"
#include "sprites.h"
#include "types.h"
#include <Arduino.h>
#include <app.hpp>
#include <cmath>
#include <etl/algorithm.h>
#include <etl/to_string.h>
#include <etl/utility.h>

#include <kbd/sdl_key.hpp>
#include <logo/doom.hpp>
#include <sdl/sdl_like.hpp>

#include <doom_impl.hpp>

#define sign(a, b) (double)(a > b ? 1 : (b > a ? -1 : 0))

// general
uint8_t scene = INTRO;
bool exit_scene = false;
bool invert_screen = false;
uint8_t flash_screen = 0;

// game
// player and entities
Player player;
Entity entity[MAX_ENTITIES];
StaticEntity static_entity[MAX_STATIC_ENTITIES];
uint8_t num_entities = 0;
uint8_t num_static_entities = 0;

// Jump to another scene
void jumpTo(uint8_t target_scene) {
  scene = target_scene;
  exit_scene = true;
}

// Finds the player in the map
void initializeLevel(const uint8_t level[]) {
  for (uint8_t y = LEVEL_HEIGHT - 1; y >= 0; y--) {
    for (uint8_t x = 0; x < LEVEL_WIDTH; x++) {
      uint8_t block = getBlockAt(level, x, y);

      if (block == E_PLAYER) {
        player = create_player(x, y);
        return;
      }

      // todo create other static entities
    }
  }
}

uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y) {
  if (x < 0 || x >= LEVEL_WIDTH || y < 0 || y >= LEVEL_HEIGHT) {
    return E_FLOOR;
  }

  // y is read in inverse order
  return pgm_read_byte(level +
                       (((LEVEL_HEIGHT - 1 - y) * LEVEL_WIDTH + x) / 2)) >>
             (!(x % 2) * 4) // displace part of wanted bits
         & 0b1111;          // mask wanted bits
}

bool isSpawned(UID uid) {
  for (uint8_t i = 0; i < num_entities; i++) {
    if (entity[i].uid == uid)
      return true;
  }

  return false;
}

bool isStatic(UID uid) {
  for (uint8_t i = 0; i < num_static_entities; i++) {
    if (static_entity[i].uid == uid)
      return true;
  }

  return false;
}

void spawnEntity(uint8_t type, uint8_t x, uint8_t y) {
  // Limit the number of spawned entities
  if (num_entities >= MAX_ENTITIES) {
    return;
  }

  // todo: read static entity status

  switch (type) {
  case E_ENEMY:
    entity[num_entities] = create_enemy(x, y);
    num_entities++;
    break;

  case E_KEY:
    entity[num_entities] = create_key(x, y);
    num_entities++;
    break;

  case E_MEDIKIT:
    entity[num_entities] = create_medikit(x, y);
    num_entities++;
    break;
  }
}

void spawnFireball(double x, double y) {
  // Limit the number of spawned entities
  if (num_entities >= MAX_ENTITIES) {
    return;
  }

  UID uid = create_uid(E_FIREBALL, x, y);
  // Remove if already exists, don't throw anything. Not the best, but shouldn't
  // happen too often
  if (isSpawned(uid))
    return;

  // Calculate direction. 32 angles
  int16_t dir = FIREBALL_ANGLES + atan2(y - player.pos.y, x - player.pos.x) /
                                      PI * FIREBALL_ANGLES;
  if (dir < 0)
    dir += FIREBALL_ANGLES * 2;
  entity[num_entities] = create_fireball(x, y, dir);
  num_entities++;
}

void removeEntity(UID uid, bool makeStatic = false) {
  uint8_t i = 0;
  bool found = false;

  while (i < num_entities) {
    if (!found && entity[i].uid == uid) {
      // todo: doze it
      found = true;
      num_entities--;
    }

    // displace entities
    if (found) {
      entity[i] = entity[i + 1];
    }

    i++;
  }
}

void removeStaticEntity(UID uid) {
  uint8_t i = 0;
  bool found = false;

  while (i < num_static_entities) {
    if (!found && static_entity[i].uid == uid) {
      found = true;
      num_static_entities--;
    }

    // displace entities
    if (found) {
      static_entity[i] = static_entity[i + 1];
    }

    i++;
  }
}

UID detectCollision(const uint8_t level[], Coords *pos, double relative_x,
                    double relative_y, bool only_walls = false) {
  // Wall collision
  uint8_t round_x = int(pos->x + relative_x);
  uint8_t round_y = int(pos->y + relative_y);
  uint8_t block = getBlockAt(level, round_x, round_y);

  if (block == E_WALL) {
    return create_uid(block, round_x, round_y);
  }

  if (only_walls) {
    return UID_null;
  }

  // Entity collision
  for (uint8_t i = 0; i < num_entities; i++) {
    // Don't collide with itself
    if (&(entity[i].pos) == pos) {
      continue;
    }

    uint8_t type = uid_get_type(entity[i].uid);

    // Only ALIVE enemy collision
    if (type != E_ENEMY || entity[i].state == S_DEAD ||
        entity[i].state == S_HIDDEN) {
      continue;
    }

    Coords new_coords = {entity[i].pos.x - relative_x,
                         entity[i].pos.y - relative_y};
    uint8_t distance = coords_distance(pos, &new_coords);

    // Check distance and if it's getting closer
    if (distance < ENEMY_COLLIDER_DIST && distance < entity[i].distance) {
      return entity[i].uid;
    }
  }

  return UID_null;
}

// Shoot
void fire() {
  for (uint8_t i = 0; i < num_entities; i++) {
    // Shoot only ALIVE enemies
    if (uid_get_type(entity[i].uid) != E_ENEMY || entity[i].state == S_DEAD ||
        entity[i].state == S_HIDDEN) {
      continue;
    }

    Coords transform = translateIntoView(&(entity[i].pos));
    if (abs(transform.x) < 20 && transform.y > 0) {
      uint8_t damage = (double)etl::min(
          GUN_MAX_DAMAGE * 1.0,
          GUN_MAX_DAMAGE / (abs(transform.x) * entity[i].distance) / 5);
      if (damage > 0) {
        entity[i].health = etl::max(0, entity[i].health - damage);
        entity[i].state = S_HIT;
        entity[i].timer = 4;
      }
    }
  }
}

// Update coords if possible. Return the collided uid, if any
UID updatePosition(const uint8_t level[], Coords *pos, double relative_x,
                   double relative_y, bool only_walls = false) {
  UID collide_x = detectCollision(level, pos, relative_x, 0, only_walls);
  UID collide_y = detectCollision(level, pos, 0, relative_y, only_walls);

  if (!collide_x)
    pos->x += relative_x;
  if (!collide_y)
    pos->y += relative_y;

  return collide_x || collide_y || UID_null;
}

void updateEntities(const uint8_t level[]) {
  uint8_t i = 0;
  while (i < num_entities) {
    // update distance
    entity[i].distance = coords_distance(&(player.pos), &(entity[i].pos));

    // Run the timer. Works with actual frames.
    // Todo: use delta here. But needs double type and more memory
    if (entity[i].timer > 0)
      entity[i].timer--;

    // too far away. put it in doze mode
    if (entity[i].distance > MAX_ENTITY_DISTANCE) {
      removeEntity(entity[i].uid);
      // don't increase 'i', since current one has been removed
      continue;
    }

    // bypass render if hidden
    if (entity[i].state == S_HIDDEN) {
      i++;
      continue;
    }

    uint8_t type = uid_get_type(entity[i].uid);

    switch (type) {
    case E_ENEMY: {
      // Enemy "IA"
      if (entity[i].health == 0) {
        if (entity[i].state != S_DEAD) {
          entity[i].state = S_DEAD;
          entity[i].timer = 6;
        }
      } else if (entity[i].state == S_HIT) {
        if (entity[i].timer == 0) {
          // Back to alert state
          entity[i].state = S_ALERT;
          entity[i].timer = 40; // delay next fireball thrown
        }
      } else if (entity[i].state == S_FIRING) {
        if (entity[i].timer == 0) {
          // Back to alert state
          entity[i].state = S_ALERT;
          entity[i].timer = 40; // delay next fireball throwm
        }
      } else {
        // ALERT STATE
        if (entity[i].distance > ENEMY_MELEE_DIST &&
            entity[i].distance < MAX_ENEMY_VIEW) {
          if (entity[i].state != S_ALERT) {
            entity[i].state = S_ALERT;
            entity[i].timer = 20; // used to throw fireballs
          } else {
            if (entity[i].timer == 0) {
              // Throw a fireball
              spawnFireball(entity[i].pos.x, entity[i].pos.y);
              entity[i].state = S_FIRING;
              entity[i].timer = 6;
            } else {
              // move towards to the player.
              updatePosition(
                  level, &(entity[i].pos),
                  sign(player.pos.x, entity[i].pos.x) * ENEMY_SPEED * delta,
                  sign(player.pos.y, entity[i].pos.y) * ENEMY_SPEED * delta,
                  true);
            }
          }
        } else if (entity[i].distance <= ENEMY_MELEE_DIST) {
          if (entity[i].state != S_MELEE) {
            // Preparing the melee attack
            entity[i].state = S_MELEE;
            entity[i].timer = 10;
          } else if (entity[i].timer == 0) {
            // Melee attack
            player.health = etl::max(0, player.health - ENEMY_MELEE_DAMAGE);
            entity[i].timer = 14;
            flash_screen = 1;
            updateHud();
          }
        } else {
          // stand
          entity[i].state = S_STAND;
        }
      }
      break;
    }

    case E_FIREBALL: {
      if (entity[i].distance < FIREBALL_COLLIDER_DIST) {
        // Hit the player and disappear
        player.health = etl::max(0, player.health - ENEMY_FIREBALL_DAMAGE);
        flash_screen = 1;
        updateHud();
        removeEntity(entity[i].uid);
        continue; // continue in the loop
      } else {
        // Move. Only collide with walls.
        // Note: using health to store the angle of the movement
        UID collided = updatePosition(
            level, &(entity[i].pos),
            cos((double)entity[i].health / FIREBALL_ANGLES * PI) *
                FIREBALL_SPEED,
            sin((double)entity[i].health / FIREBALL_ANGLES * PI) *
                FIREBALL_SPEED,
            true);

        if (collided) {
          removeEntity(entity[i].uid);
          continue; // continue in the entity check loop
        }
      }
      break;
    }

    case E_MEDIKIT: {
      if (entity[i].distance < ITEM_COLLIDER_DIST) {
        // pickup
        entity[i].state = S_HIDDEN;
        player.health = etl::min(100, player.health + 50);
        updateHud();
        flash_screen = 1;
      }
      break;
    }

    case E_KEY: {
      if (entity[i].distance < ITEM_COLLIDER_DIST) {
        // pickup
        entity[i].state = S_HIDDEN;
        player.keys++;
        updateHud();
        flash_screen = 1;
      }
      break;
    }
    }

    i++;
  }
}

// The map raycaster. Based on https://lodev.org/cgtutor/raycasting.html
void renderMap(const uint8_t level[], double view_height) {
  UID last_uid;

  for (uint8_t x = 0; x < SCREEN_WIDTH; x += RES_DIVIDER) {
    double camera_x = 2 * (double)x / SCREEN_WIDTH - 1;
    double ray_x = player.dir.x + player.plane.x * camera_x;
    double ray_y = player.dir.y + player.plane.y * camera_x;
    uint8_t map_x = uint8_t(player.pos.x);
    uint8_t map_y = uint8_t(player.pos.y);
    Coords map_coords = {player.pos.x, player.pos.y};
    double delta_x = std::abs(1 / ray_x);
    double delta_y = std::abs(1 / ray_y);

    int8_t step_x;
    int8_t step_y;
    double side_x;
    double side_y;

    if (ray_x < 0) {
      step_x = -1;
      side_x = (player.pos.x - map_x) * delta_x;
    } else {
      step_x = 1;
      side_x = (map_x + 1.0 - player.pos.x) * delta_x;
    }

    if (ray_y < 0) {
      step_y = -1;
      side_y = (player.pos.y - map_y) * delta_y;
    } else {
      step_y = 1;
      side_y = (map_y + 1.0 - player.pos.y) * delta_y;
    }

    // Wall detection
    uint8_t depth = 0;
    bool hit = 0;
    bool side;
    while (!hit && depth < MAX_RENDER_DEPTH) {
      if (side_x < side_y) {
        side_x += delta_x;
        map_x += step_x;
        side = 0;
      } else {
        side_y += delta_y;
        map_y += step_y;
        side = 1;
      }

      uint8_t block = getBlockAt(level, map_x, map_y);

      if (block == E_WALL) {
        hit = 1;
      } else {
        // Spawning entities here, as soon they are visible for the
        // player. Not the best place, but would be a very performance
        // cost scan for them in another loop
        if (block == E_ENEMY ||
            (block & 0b00001000) /* all collectable items */) {
          // Check that it's close to the player
          if (coords_distance(&(player.pos), &map_coords) <
              MAX_ENTITY_DISTANCE) {
            UID uid = create_uid(block, map_x, map_y);
            if (last_uid != uid && !isSpawned(uid)) {
              spawnEntity(block, map_x, map_y);
              last_uid = uid;
            }
          }
        }
      }

      depth++;
    }

    if (hit) {
      double distance;

      if (side == 0) {
        distance =
            etl::max(1.0, (map_x - player.pos.x + (1 - step_x) / 2) / ray_x);
      } else {
        distance =
            etl::max(1.0, (map_y - player.pos.y + (1 - step_y) / 2) / ray_y);
      }

      // store zbuffer value for the column
      zbuffer[x / Z_RES_DIVIDER] =
          etl::min(distance * DISTANCE_MULTIPLIER, 255.0);

      // rendered line height
      uint8_t line_height = RENDER_HEIGHT / distance;

      drawVLine(x, view_height / distance - line_height / 2 + RENDER_HEIGHT / 2,
                view_height / distance + line_height / 2 + RENDER_HEIGHT / 2,
                GRADIENT_COUNT -
                    int(distance / MAX_RENDER_DEPTH * GRADIENT_COUNT) -
                    side * 2);
    }
  }
}

// Sort entities from far to close
uint8_t sortEntities() {
  uint8_t gap = num_entities;
  bool swapped = false;
  while (gap > 1 || swapped) {
    // shrink factor 1.3
    gap = (gap * 10) / 13;
    if (gap == 9 || gap == 10)
      gap = 11;
    if (gap < 1)
      gap = 1;
    swapped = false;
    for (uint8_t i = 0; i < num_entities - gap; i++) {
      uint8_t j = i + gap;
      if (entity[i].distance < entity[j].distance) {
        std::swap(entity[i], entity[j]);
        swapped = true;
      }
    }
  }
  return 0;
}

Coords translateIntoView(Coords *pos) {
  // translate sprite position to relative to camera
  double sprite_x = pos->x - player.pos.x;
  double sprite_y = pos->y - player.pos.y;

  // required for correct matrix multiplication
  double inv_det =
      1.0 / (player.plane.x * player.dir.y - player.dir.x * player.plane.y);
  double transform_x =
      inv_det * (player.dir.y * sprite_x - player.dir.x * sprite_y);
  double transform_y = inv_det * (-player.plane.y * sprite_x +
                                  player.plane.x * sprite_y); // Z in screen

  return {transform_x, transform_y};
}

void renderEntities(double view_height) {
  sortEntities();

  for (uint8_t i = 0; i < num_entities; i++) {
    if (entity[i].state == S_HIDDEN)
      continue;

    Coords transform = translateIntoView(&(entity[i].pos));

    // don´t render if behind the player or too far away
    if (transform.y <= 0.1 || transform.y > MAX_SPRITE_DEPTH) {
      continue;
    }

    int16_t sprite_screen_x = HALF_WIDTH * (1.0 + transform.x / transform.y);
    int8_t sprite_screen_y = RENDER_HEIGHT / 2 + view_height / transform.y;
    uint8_t type = uid_get_type(entity[i].uid);

    // don´t try to render if outside of screen
    // doing this pre-shortcut due int16 -> int8 conversion makes out-of-screen
    // values fit into the screen space
    if (sprite_screen_x < -HALF_WIDTH ||
        sprite_screen_x > SCREEN_WIDTH + HALF_WIDTH) {
      continue;
    }

    switch (type) {
    case E_ENEMY: {
      uint8_t sprite;
      if (entity[i].state == S_ALERT) {
        // walking
        sprite = int(millis() / 500) % 2;
      } else if (entity[i].state == S_FIRING) {
        // fireball
        sprite = 2;
        drawSprite(sprite_screen_x - kDemonWidth * .5 / transform.y,
                   sprite_screen_y - 8 / transform.y, kDoomDemonAttack,
                   kDemonWidth, kDemonHeight, transform.y);
        break;

      } else if (entity[i].state == S_HIT) {
        // hit
        sprite = 3;
      } else if (entity[i].state == S_MELEE) {
        // melee atack
        sprite = entity[i].timer > 10 ? 2 : 1;
      } else if (entity[i].state == S_DEAD) {
        // dying
        sprite = entity[i].timer > 0 ? 3 : 4;
        drawSprite(sprite_screen_x - BMP_IMP_WIDTH * .5 / transform.y,
                   sprite_screen_y - 8 / transform.y, bmp_imp_bits,
                   bmp_imp_mask, BMP_IMP_WIDTH, BMP_IMP_HEIGHT, sprite,
                   transform.y);
        break;
      } else {
        // stand
        sprite = 0;
      }

      drawSprite(sprite_screen_x - kDemonWidth * .5 / transform.y,
                 sprite_screen_y - 8 / transform.y, kDoomDemon, kDemonWidth,
                 kDemonHeight, transform.y);

      break;
    }

    case E_FIREBALL: {
      drawSprite(sprite_screen_x +
                     (kDemonWidth - BMP_FIREBALL_WIDTH * 2) / 2 / transform.y,
                 sprite_screen_y +
                     (kDemonHeight - BMP_FIREBALL_HEIGHT) / 2 / transform.y,
                 bmp_fireball_bits, bmp_fireball_mask, BMP_FIREBALL_WIDTH,
                 BMP_FIREBALL_HEIGHT, 0, transform.y);
      break;
    }

    case E_MEDIKIT: {
      drawSprite(sprite_screen_x - BMP_ITEMS_WIDTH / 2 / transform.y,
                 sprite_screen_y + 5 / transform.y, bmp_items_bits,
                 bmp_items_mask, BMP_ITEMS_WIDTH, BMP_ITEMS_HEIGHT, 0,
                 transform.y);
      break;
    }

    case E_KEY: {
      drawSprite(sprite_screen_x - BMP_ITEMS_WIDTH / 2 / transform.y,
                 sprite_screen_y + 5 / transform.y, bmp_items_bits,
                 bmp_items_mask, BMP_ITEMS_WIDTH, BMP_ITEMS_HEIGHT, 1,
                 transform.y);
      break;
    }
    }
  }
}

void renderGun(uint8_t gun_pos, double amount_jogging) {
  // jogging
  char x = 48 + sin((double)millis() * JOGGING_SPEED) * 10 * amount_jogging;
  char y = RENDER_HEIGHT - gun_pos +
           abs(cos((double)millis() * JOGGING_SPEED)) * 8 * amount_jogging;

  if (gun_pos > GUN_SHOT_POS - 2) {
    // Gun fire
    drawBitmap(x + 6, y - 11, bmp_fire_bits, BMP_FIRE_WIDTH, BMP_FIRE_HEIGHT,
               1);
  }

  // Don't draw over the hud!
  uint8_t clip_height =
      etl::max(0, etl::min(y + BMP_GUN_HEIGHT, RENDER_HEIGHT) - y);

  // Draw the gun (black mask + actual sprite).
  drawBitmap(x, y, bmp_gun_mask, BMP_GUN_WIDTH, clip_height, 0);
  drawBitmap(x, y, bmp_gun_bits, BMP_GUN_WIDTH, clip_height, 1);
}

// Only needed first time
void renderHud() {
  drawText(2, RENDER_HEIGHT + sdl::kFontHeight, "+");   // Health symbol
  drawText(38, RENDER_HEIGHT + sdl::kFontHeight, "[]"); // Keys symbol
  updateHud();
}

// Render values for the HUD
void updateHud() {
  etl::string<10> health_str;
  etl::to_string(player.health, health_str);

  drawText(12, RENDER_HEIGHT + sdl::kFontHeight, health_str.c_str());
  etl::to_string(player.keys, health_str);
  drawText(50, RENDER_HEIGHT + sdl::kFontHeight, health_str.c_str());
}

// Debug stats
void renderStats() {
  etl::string<10> str_buffer;
  etl::to_string(30, str_buffer);

  drawText(114, RENDER_HEIGHT + sdl::kFontHeight, str_buffer.c_str());
  etl::to_string(num_entities, str_buffer);
  drawText(82, RENDER_HEIGHT + sdl::kFontHeight, str_buffer.c_str());
}

// Intro screen
void loopIntro(sdl::Key key) {
  static long time = millis();
  static long last_frame_time = 0;
  static int frame = 0;

  auto current_time = millis();

  if (current_time - last_frame_time > gf::kFrameDelay) {
    sdl::Sprite()->pushImage(
        (SCREEN_WIDTH - gf::kDoomLogoWidth) / 2,
        (SCREEN_HEIGHT - gf::kDoomLogoHeight) / 4, gf::kDoomLogoWidth,
        gf::kDoomLogoHeight,
        gf::kDoomLogoFrame[frame++ % gf::kDoomLogoFrame.size()]);

    last_frame_time = current_time;
  }

  // drawBitmap((SCREEN_WIDTH - BMP_LOGO_WIDTH) / 2,
  //            (SCREEN_HEIGHT - BMP_LOGO_HEIGHT) / 3, bmp_logo_bits,
  //            BMP_LOGO_WIDTH, BMP_LOGO_HEIGHT, 1);

  if (current_time - time > 1000) {
    drawText((sdl::kWidth - sizeof("Press Fire") * (sdl::kFontWidth + 1)) / 2,
             RENDER_HEIGHT, "Press Fire", sdl::kDoomOrange);
  }

  sdl::PushSprite(0, 0);

  // wait for fire
  if (key == sdl::KeySelect)
    jumpTo(GAME_PLAY);
}

void loopGamePlay(sdl::Key key) {
  static bool gun_fired = false;
  static uint8_t gun_pos = 0;
  static double rot_speed;
  static double old_dir_x;
  static double old_plane_x;
  static double view_height;
  static double jogging;
  static uint8_t fade = GRADIENT_COUNT - 1;

  static bool init = false;
  if (!init) {
    initializeLevel(sto_level_1);
    init = true;
  }

  // Clear only the 3d view

  sdl::Sprite()->fillScreen(sdl::Color::kBlack);

  // If the player is alive
  if (player.health > 0) {
    // Player speed
    if (key == sdl::KeyUp) {
      player.velocity += (MOV_SPEED - player.velocity) * .4;
      jogging = std::abs(player.velocity) * MOV_SPEED_INV;
    } else if (key == sdl::KeyDown) {
      player.velocity += (-MOV_SPEED - player.velocity) * .4;
      jogging = std::abs(player.velocity) * MOV_SPEED_INV;
    } else {
      player.velocity *= .5;
      jogging = std::abs(player.velocity) * MOV_SPEED_INV;
    }

    // Player rotation
    if (key == sdl::KeyRight) {
      rot_speed = ROT_SPEED * delta;
      old_dir_x = player.dir.x;
      player.dir.x = player.dir.x * std::cos(-rot_speed) -
                     player.dir.y * std::sin(-rot_speed);
      player.dir.y = old_dir_x * std::sin(-rot_speed) +
                     player.dir.y * std::cos(-rot_speed);
      old_plane_x = player.plane.x;
      player.plane.x = player.plane.x * std::cos(-rot_speed) -
                       player.plane.y * std::sin(-rot_speed);
      player.plane.y =
          old_plane_x * std::sin(-rot_speed) + player.plane.y * cos(-rot_speed);
    } else if (key == sdl::KeyLeft) {
      rot_speed = ROT_SPEED * delta;
      old_dir_x = player.dir.x;
      player.dir.x =
          player.dir.x * cos(rot_speed) - player.dir.y * sin(rot_speed);
      player.dir.y = old_dir_x * sin(rot_speed) + player.dir.y * cos(rot_speed);
      old_plane_x = player.plane.x;
      player.plane.x =
          player.plane.x * cos(rot_speed) - player.plane.y * sin(rot_speed);
      player.plane.y =
          old_plane_x * sin(rot_speed) + player.plane.y * cos(rot_speed);
    }

    view_height = abs(sin((double)millis() * JOGGING_SPEED)) * 6 * jogging;
    // Update gun
    if (gun_pos > GUN_TARGET_POS) {
      // Right after fire
      gun_pos -= 1;
    } else if (gun_pos < GUN_TARGET_POS) {
      // Showing up
      gun_pos += 2;
    } else if (!gun_fired && key == sdl::KeySelect) {
      // ready to fire and fire pressed
      gun_pos = GUN_SHOT_POS;
      gun_fired = true;
      fire();
    } else if (gun_fired && !(key == sdl::KeySelect)) {
      // just fired and restored position
      gun_fired = false;
    }
  } else {
    // The player is dead
    if (view_height > -10)
      view_height--;
    else if (key == sdl::KeyBack)
      jumpTo(INTRO);

    if (gun_pos > 1)
      gun_pos -= 2;
  }

  // Player movement
  if (abs(player.velocity) > 0.003) {
    updatePosition(sto_level_1, &(player.pos),
                   player.dir.x * player.velocity * delta,
                   player.dir.y * player.velocity * delta);
  } else {
    player.velocity = 0;
  }

  // Update things
  updateEntities(sto_level_1);

  // Render stuff
  renderMap(sto_level_1, view_height);
  renderEntities(view_height);
  renderGun(gun_pos, jogging);

  renderHud();
  renderStats();

  // if (fade > 0) {
  //   fadeScreen(fade);
  //   fade--;

  //   if (fade == 0) {
  //     // Only draw the hud after fade in effect
  //     renderHud();
  //   }
  // } else {
  //
  // }

  // flash screen
  // if (flash_screen > 0) {
  //   invert_screen = !invert_screen;
  //   flash_screen--;
  // } else if (invert_screen) {
  //   invert_screen = 0;
  // }

  // Draw the frame
  sdl::PushSprite(0, 0);

  if (key == sdl::KeyBack) {
    jumpTo(INTRO);
  }
}

void DoomInit() {
  setupDisplay();
  sdl::AllocSprite(sdl::kWidth, sdl::kHeight, 8);
  // sdl::Sprite()->createPalette(gf::kColorPallete.data());
  // TODO(egor): add color palette
  // sdl::Sprite()
}

AppsList DoomStep(sdl::Key key) {
  switch (scene) {
  case INTRO: {
    loopIntro(key);
    break;
  }
  case GAME_PLAY: {
    loopGamePlay(key);
    break;
  }
  }

  // fade out effect
  for (uint8_t i = 0; i < GRADIENT_COUNT; i++) {
    // fadeScreen(i, 0);
    // display.display();
    // delay(40);
  }
  exit_scene = false;
  return kDoom;
}
