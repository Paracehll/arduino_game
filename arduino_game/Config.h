#ifndef CONFIG_H
#define CONFIG_H

// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// TFT SPI Pins
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

// Button Pins for Movement
#define BUTTON_LEFT_PIN  4
#define BUTTON_RIGHT_PIN 5
#define BUTTON_UP_PIN    6
#define BUTTON_DOWN_PIN  7

// Joystick Pins (X only now for optional fallback or reference)
#define JOY_X_PIN A0

// Button Pins
#define BUTTON_SHOOT_PIN 2
#define BUTTON_BOMB_PIN  3

// Game Constants (Reduced for Arduino Uno RAM limits)
#define MAX_PLAYER_BULLETS 8
#define MAX_ENEMY_BULLETS  12
#define MAX_ENEMIES        6
#define MAX_POWERUPS       3
#define PLAYER_SPEED       4
#define BULLET_SPEED       6
#define ENEMY_SPEED        2
#define ENEMY_STOP_SPEED   0.5
#define ENEMY_BULLET_SPEED 3
#define PLAYER_Y           280 // Initial Y position
#define INITIAL_LIVES      3

// Enemy States
#define STATE_NORMAL       0
#define STATE_STOP         1

// Combat Balancing
#define BULLET_DAMAGE      1
#define ENEMY_DEFAULT_HP   3  // Neutral units have higher health
#define ENEMY_HIGH_HP      10 // High-level units have even higher health

#endif
