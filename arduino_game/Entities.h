#ifndef ENTITIES_H
#define ENTITIES_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

struct Player {
    float x, y;
    int score;
    int bombs;
    int lives;
    int powerLevel;
    bool alive;
    int width, height;
};

struct Bullet {
    float x, y;
    bool active;
    int width, height;
};

struct Enemy {
    float x, y;
    bool active;
    bool transferred;
    bool isHighLevel;
    int hp;
    int width, height;
    int type;
};

struct PowerUp {
    float x, y;
    bool active;
    int width, height;
};

#endif
