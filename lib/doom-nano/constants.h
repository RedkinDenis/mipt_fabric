#ifndef _constants_h
#define _constants_h

#include <stdint.h>

#define OPTIMIZE_SSD1306 // Optimizations for SSD1366 displays

#define FRAME_TIME                                                             \
  66.666666 // Desired time per frame in ms (66.666666 is ~15 fps)
#define RES_DIVIDER                                                            \
  1 // Higher values will result in lower horizontal resolution when rasterize
    // and lower process and memory usage Lower will require more process and
    // memory, but looks nicer
#define Z_RES_DIVIDER                                                          \
  1 // Zbuffer resolution divider. We sacrifice resolution to save memory
#define DISTANCE_MULTIPLIER                                                    \
  12 // Distances are stored as uint8_t, multiplying the distance we can obtain
     // more precision taking care of keep numbers inside the type range. Max is
     // 256 / MAX_RENDER_DEPTH
#define MAX_RENDER_DEPTH 12
#define MAX_SPRITE_DEPTH 8

#define ZBUFFER_SIZE SCREEN_WIDTH / Z_RES_DIVIDER

// Level
#define LEVEL_WIDTH_BASE 6
#define LEVEL_WIDTH (1 << LEVEL_WIDTH_BASE)
#define LEVEL_HEIGHT 57
#define LEVEL_SIZE LEVEL_WIDTH / 2 * LEVEL_HEIGHT

// scenes
#define INTRO 0
#define GAME_PLAY 1

// Game
#define GUN_TARGET_POS 18
#define GUN_SHOT_POS GUN_TARGET_POS + 4

#define ROT_SPEED .12
#define MOV_SPEED .2
#define MOV_SPEED_INV 5 // 1 / MOV_SPEED

#define JOGGING_SPEED .005
#define ENEMY_SPEED .02
#define FIREBALL_SPEED .2
#define FIREBALL_ANGLES 45 // Num of angles per PI

#define MAX_ENTITIES 10        // Max num of active entities
#define MAX_STATIC_ENTITIES 28 // Max num of entities in sleep mode

#define MAX_ENTITY_DISTANCE 200  // * DISTANCE_MULTIPLIER
#define MAX_ENEMY_VIEW 80        // * DISTANCE_MULTIPLIER
#define ITEM_COLLIDER_DIST 6     // * DISTANCE_MULTIPLIER
#define ENEMY_COLLIDER_DIST 4    // * DISTANCE_MULTIPLIER
#define FIREBALL_COLLIDER_DIST 2 // * DISTANCE_MULTIPLIER
#define ENEMY_MELEE_DIST 6       // * DISTANCE_MULTIPLIER
#define WALL_COLLIDER_DIST .2

#define ENEMY_MELEE_DAMAGE 8
#define ENEMY_FIREBALL_DAMAGE 20
#define GUN_MAX_DAMAGE 15

// display
constexpr uint8_t SCREEN_WIDTH = 160;
constexpr uint8_t SCREEN_HEIGHT = 128;
constexpr uint8_t HALF_WIDTH = SCREEN_WIDTH / 2;

// raycaster working height (the rest is for the hud)
constexpr int RENDER_HEIGHT = 100;
constexpr uint8_t HALF_HEIGHT = SCREEN_HEIGHT / 2;

#endif
