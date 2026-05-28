#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Config.h"
#include "Entities.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

void updatePlayer(Player &p) {
    static int lastA = LOW;
    int currentA = digitalRead(ENC_A_PIN);

    if (currentA != lastA && currentA == HIGH) {
        if (digitalRead(ENC_B_PIN) != currentA) {
            p.x += 10; // Adjust sensitivity
        } else {
            p.x -= 10;
        }
    }
    lastA = currentA;

    // Fixed Y
    p.y = PLAYER_Y;

    // Constrain to screen
    if (p.x < 0) p.x = 0;
    if (p.x > SCREEN_WIDTH - p.width) p.x = SCREEN_WIDTH - p.width;
}

void fireBullet(Player &p, Bullet bullets[]) {
    int count = 1 + p.powerLevel;
    if (count > 3) count = 3; // Limit spread

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PLAYER_BULLETS; j++) {
            if (!bullets[j].active) {
                bullets[j].width = 4;
                bullets[j].height = 10;
                bullets[j].active = true;
                bullets[j].y = p.y;

                if (count == 1) {
                    bullets[j].x = p.x + p.width / 2 - bullets[j].width / 2;
                } else if (count == 2) {
                    bullets[j].x = (i == 0) ? p.x : p.x + p.width - bullets[j].width;
                } else {
                    if (i == 0) bullets[j].x = p.x;
                    else if (i == 1) bullets[j].x = p.x + p.width / 2 - bullets[j].width / 2;
                    else bullets[j].x = p.x + p.width - bullets[j].width;
                }
                break;
            }
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

            // Randomly spawn high level if not transferred
            if (!transferred && random(0, 10) > 7) {
                enemies[i].isHighLevel = true;
                enemies[i].hp = 3;
            } else {
                enemies[i].isHighLevel = false;
                enemies[i].hp = 1;
            }
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
