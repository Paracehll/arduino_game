#ifndef CONFIG_H
#define CONFIG_H

// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// TFT SPI Pins
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
// Note: Hardware SPI uses 11 (MOSI) and 13 (SCK) on Arduino Uno

// Joystick Pins
#define JOY_X_PIN A0
#define JOY_Y_PIN A1

// Button Pins
#define BUTTON_SHOOT_PIN 2
#define BUTTON_BOMB_PIN  3

// Game Constants
#define MAX_PLAYER_BULLETS 10
#define MAX_ENEMIES        15
#define PLAYER_SPEED       4
#define BULLET_SPEED       6
#define ENEMY_SPEED        2

#endif
