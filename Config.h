#ifndef CONFIG_H
#define CONFIG_H

// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// TFT SPI Pins
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

// Optical Encoder Pins (PEC16 16 mm Incremental Encoder)
#define ENC_A_PIN 4
#define ENC_B_PIN 5

// Joystick Pins (X only now for optional fallback or reference, but we use Encoder)
#define JOY_X_PIN A0

// Button Pins
#define BUTTON_SHOOT_PIN 2
#define BUTTON_BOMB_PIN  3

// Game Constants
#define MAX_PLAYER_BULLETS 20
#define MAX_ENEMIES        15
#define MAX_POWERUPS       5
#define PLAYER_SPEED       4
#define BULLET_SPEED       6
#define ENEMY_SPEED        2
#define PLAYER_Y           280 // Fixed Y position
#define INITIAL_LIVES      3

// Combat Balancing
#define BULLET_DAMAGE      1
#define ENEMY_DEFAULT_HP   3  // Neutral units have higher health
#define ENEMY_HIGH_HP      10 // High-level units have even higher health

#endif
