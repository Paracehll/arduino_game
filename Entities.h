#ifndef ENTITIES_H
#define ENTITIES_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

struct Player {
    float x, y;
    int score;
    int bombs;
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
    bool transferred; // true if this enemy has already been transferred (received from opponent)
    int width, height;
    int type; // Can define different enemy types if needed
};

#endif
