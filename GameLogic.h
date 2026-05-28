#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Config.h"
#include "Entities.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

void updatePlayer(Player &p) {
    int joyX = analogRead(JOY_X_PIN);
    int joyY = analogRead(JOY_Y_PIN);

    // Assuming joystick center is around 512
    if (joyX < 400) p.x -= PLAYER_SPEED;
    if (joyX > 600) p.x += PLAYER_SPEED;
    if (joyY < 400) p.y -= PLAYER_SPEED;
    if (joyY > 600) p.y += PLAYER_SPEED;

    // Constrain to screen
    if (p.x < 0) p.x = 0;
    if (p.x > SCREEN_WIDTH - p.width) p.x = SCREEN_WIDTH - p.width;
    if (p.y < 0) p.y = 0;
    if (p.y > SCREEN_HEIGHT - p.height) p.y = SCREEN_HEIGHT - p.height;
}

void fireBullet(Player &p, Bullet bullets[]) {
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = p.x + p.width / 2 - bullets[i].width / 2;
            bullets[i].y = p.y;
            bullets[i].active = true;
            break;
        }
    }
}

void updateBullets(Bullet bullets[]) {
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= BULLET_SPEED;
            if (bullets[i].y < 0) {
                bullets[i].active = false;
            }
        }
    }
}

void spawnEnemy(Enemy enemies[], float x, bool transferred) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].x = x;
            enemies[i].y = 0;
            enemies[i].active = true;
            enemies[i].transferred = transferred;
            enemies[i].width = 20;
            enemies[i].height = 20;
            break;
        }
    }
}

void updateEnemies(Enemy enemies[]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].y += ENEMY_SPEED;
            if (enemies[i].y > SCREEN_HEIGHT) {
                enemies[i].active = false;
            }
        }
    }
}

bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

#endif
