#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Config.h"
#include "Entities.h"
#include <math.h>
#ifdef ARDUINO
#include <Arduino.h>
#endif

void updatePlayer(Player &p) {
    if (digitalRead(BUTTON_LEFT_PIN) == LOW) {
        p.x -= PLAYER_SPEED;
    }
    if (digitalRead(BUTTON_RIGHT_PIN) == LOW) {
        p.x += PLAYER_SPEED;
    }
    if (digitalRead(BUTTON_UP_PIN) == LOW) {
        p.y -= PLAYER_SPEED;
    }
    if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
        p.y += PLAYER_SPEED;
    }

    // Constrain X to screen
    if (p.x < 0) p.x = 0;
    if (p.x > SCREEN_WIDTH - p.width) p.x = SCREEN_WIDTH - p.width;

    // Constrain Y to screen (allow movement in bottom 3/4 of screen)
    if (p.y < SCREEN_HEIGHT / 4) p.y = SCREEN_HEIGHT / 4;
    if (p.y > SCREEN_HEIGHT - p.height) p.y = SCREEN_HEIGHT - p.height;
}

void fireBullet(Player &p, Bullet bullets[]) {
    static uint16_t lastPlayerFire = 0;
    uint16_t now = (uint16_t)millis();
    if (now - lastPlayerFire < PLAYER_FIRE_COOLDOWN) return;
    lastPlayerFire = now;

    int8_t count = 1 + p.powerLevel;
    if (count > 3) count = 3; // Limit spread

    for (int8_t i = 0; i < count; i++) {
        for (int8_t j = 0; j < MAX_PLAYER_BULLETS; j++) {
            if (!bullets[j].active) {
                bullets[j].width = 4;
                bullets[j].height = 10;
                bullets[j].active = 1;
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
    for (int8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= BULLET_SPEED;
            if (bullets[i].y < 0) {
                bullets[i].active = 0;
            }
        }
    }
}

void fireEnemyBullet(Enemy &e, Player &p, EnemyBullet enemyBullets[]) {
    uint16_t now = (uint16_t)millis();
    if (now - e.lastFireTime < 1000) return; // Fire every 1 second
    e.lastFireTime = now;

    switch (e.pattern) {
        case 0: { // Aimed at player
            float dx = (p.x + p.width / 2) - (e.x + e.width / 2);
            float dy = (p.y + p.height / 2) - (e.y + e.height / 2);
            float dist = sqrt(dx * dx + dy * dy);
            if (dist == 0) dist = 1;

            for (int8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
                if (!enemyBullets[i].active) {
                    enemyBullets[i].active = 1;
                    enemyBullets[i].x = e.x + e.width / 2;
                    enemyBullets[i].y = e.y + e.height / 2;
                    enemyBullets[i].vx = (dx / dist) * ENEMY_BULLET_SPEED;
                    enemyBullets[i].vy = (dy / dist) * ENEMY_BULLET_SPEED;
                    enemyBullets[i].width = 4;
                    enemyBullets[i].height = 4;
                    break;
                }
            }
            break;
        }
        case 1: { // 3-way Spread
            for (int8_t ang = -1; ang <= 1; ang++) {
                for (int8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
                    if (!enemyBullets[i].active) {
                        enemyBullets[i].active = 1;
                        enemyBullets[i].x = e.x + e.width / 2;
                        enemyBullets[i].y = e.y + e.height / 2;
                        enemyBullets[i].vx = ang * 1.0;
                        enemyBullets[i].vy = ENEMY_BULLET_SPEED;
                        enemyBullets[i].width = 4;
                        enemyBullets[i].height = 4;
                        break;
                    }
                }
            }
            break;
        }
        default: { // 8-way Circular
            for (int8_t i = 0; i < 8; i++) {
                float angle = i * (M_PI / 4);
                for (int8_t j = 0; j < MAX_ENEMY_BULLETS; j++) {
                    if (!enemyBullets[j].active) {
                        enemyBullets[j].active = 1;
                        enemyBullets[j].x = e.x + e.width / 2;
                        enemyBullets[j].y = e.y + e.height / 2;
                        enemyBullets[j].vx = cos(angle) * ENEMY_BULLET_SPEED;
                        enemyBullets[j].vy = sin(angle) * ENEMY_BULLET_SPEED;
                        enemyBullets[j].width = 4;
                        enemyBullets[j].height = 4;
                        break;
                    }
                }
            }
            break;
        }
    }
}

void spawnEnemy(Enemy enemies[], float x, uint8_t transferred) {
    for (int8_t i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].x = x;
            enemies[i].y = 0;
            enemies[i].active = 1;
            enemies[i].transferred = transferred;
            enemies[i].width = 20;
            enemies[i].height = 20;

            // Randomly spawn high level if not transferred
            if (!transferred && random(0, 10) > 7) {
                enemies[i].isHighLevel = 1;
                enemies[i].hp = ENEMY_HIGH_HP;
            } else {
                enemies[i].isHighLevel = 0;
                enemies[i].hp = ENEMY_DEFAULT_HP;
            }

            // New state machine initialization
            enemies[i].state = STATE_NORMAL;
            enemies[i].stop_delay = random(1000, 3000);
            enemies[i].stop_timer = random(2000, 4000);
            enemies[i].stateStartTime = (uint16_t)millis();
            enemies[i].lastFireTime = 0;
            enemies[i].pattern = random(0, 3);
            break;
        }
    }
}

void updateEnemies(Enemy enemies[], Player &p, EnemyBullet enemyBullets[]) {
    uint16_t now = (uint16_t)millis();
    for (int8_t i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            uint16_t elapsed = now - enemies[i].stateStartTime;

            if (enemies[i].state == STATE_NORMAL) {
                enemies[i].y += ENEMY_SPEED;
                if (elapsed > enemies[i].stop_delay) {
                    enemies[i].state = STATE_STOP;
                    enemies[i].stateStartTime = now;
                }
            } else if (enemies[i].state == STATE_STOP) {
                enemies[i].y += ENEMY_STOP_SPEED;
                fireEnemyBullet(enemies[i], p, enemyBullets);
                if (elapsed > enemies[i].stop_timer) {
                    enemies[i].state = STATE_NORMAL;
                    enemies[i].stateStartTime = now;
                    enemies[i].stop_delay = 10000; // Only stop once
                }
            }

            if (enemies[i].y > SCREEN_HEIGHT) {
                enemies[i].active = 0;
            }
        }
    }
}

void updateEnemyBullets(EnemyBullet enemyBullets[]) {
    for (int8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemyBullets[i].active) {
            enemyBullets[i].x += enemyBullets[i].vx;
            enemyBullets[i].y += enemyBullets[i].vy;
            if (enemyBullets[i].y < 0 || enemyBullets[i].y > SCREEN_HEIGHT ||
                enemyBullets[i].x < 0 || enemyBullets[i].x > SCREEN_WIDTH) {
                enemyBullets[i].active = 0;
            }
        }
    }
}

uint8_t checkCollision(float x1, float y1, int8_t w1, int8_t h1, float x2, float y2, int8_t w2, int8_t h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

#endif
