#ifndef ENTITIES_H
#define ENTITIES_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

struct Player {
    float x, y;
    int16_t score;
    int8_t bombs;
    int8_t lives;
    int8_t powerLevel;
    uint8_t alive;
    int8_t width, height;
};

struct Bullet {
    float x, y;
    float vx; // Horizontal velocity for spread patterns
    uint8_t active;
    int8_t width, height;
};

struct EnemyBullet {
    float x, y;
    float vx, vy;
    uint8_t active;
    int8_t width, height;
};

struct Enemy {
    float x, y;
    uint8_t active;
    uint8_t transferred;
    uint8_t isHighLevel;
    int8_t hp;
    int8_t width, height;

    // New fields for neutral unit logic
    int8_t state;
    uint16_t stop_delay;
    uint16_t stop_timer;
    uint16_t stateStartTime;
    uint16_t lastFireTime;
    int8_t pattern;
};

struct PowerUp {
    float x, y;
    uint8_t active;
    int8_t width, height;
};

#endif
