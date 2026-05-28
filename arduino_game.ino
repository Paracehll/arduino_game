#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Config.h"
#include "Entities.h"
#include "Communication.h"
#include "GameLogic.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

Player player;
Bullet bullets[MAX_PLAYER_BULLETS];
Enemy enemies[MAX_ENEMIES];
PowerUp powerups[MAX_POWERUPS];

unsigned long lastSpawnTime = 0;
const unsigned long spawnInterval = 2000;
unsigned long invulnEndTime = 0;

void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(ILI9341_BLACK);

    player = {SCREEN_WIDTH / 2 - 10, PLAYER_Y, 0, 3, INITIAL_LIVES, 0, true, 20, 20};

    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    for (int i = 0; i < MAX_POWERUPS; i++) powerups[i].active = false;

    pinMode(BUTTON_SHOOT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_BOMB_PIN, INPUT_PULLUP);
    pinMode(ENC_A_PIN, INPUT_PULLUP);
    pinMode(ENC_B_PIN, INPUT_PULLUP);
}

void spawnPowerUp(float x, float y) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].active) {
            powerups[i].x = x;
            powerups[i].y = y;
            powerups[i].active = true;
            powerups[i].width = 10;
            powerups[i].height = 10;
            break;
        }
    }
}

void updatePowerUps() {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active) {
            powerups[i].y += ENEMY_SPEED;
            if (powerups[i].y > SCREEN_HEIGHT) powerups[i].active = false;
        }
    }
}

void handleCollisions() {
    bool invulnerable = millis() < invulnEndTime;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        if (!invulnerable && checkCollision(player.x, player.y, player.width, player.height,
                           enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height)) {
            player.lives--;
            if (player.lives <= 0) {
                player.alive = false;
            } else {
                invulnEndTime = millis() + 2000; // 2 seconds invulnerability
                // Optional: Clear enemies around player
                for(int k=0; k<MAX_ENEMIES; k++) enemies[k].active = false;
            }
        }

        for (int j = 0; j < MAX_PLAYER_BULLETS; j++) {
            if (bullets[j].active && checkCollision(bullets[j].x, bullets[j].y, bullets[j].width, bullets[j].height,
                                                    enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height)) {
                bullets[j].active = false;
                enemies[i].hp--;

                if (enemies[i].hp <= 0) {
                    enemies[i].active = false;
                    player.score += enemies[i].isHighLevel ? 50 : 10;

                    if (enemies[i].isHighLevel) {
                        spawnPowerUp(enemies[i].x, enemies[i].y);
                    }

                    if (!enemies[i].transferred) {
                        uint8_t x_percent = (enemies[i].x * 100) / SCREEN_WIDTH;
                        sendEnemyTransfer(x_percent);
                    }
                }
            }
        }
    }

    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active && checkCollision(player.x, player.y, player.width, player.height,
                                                 powerups[i].x, powerups[i].y, powerups[i].width, powerups[i].height)) {
            powerups[i].active = false;
            player.powerLevel++;
        }
    }
}

void drawUI() {
    static int lastScore = -1;
    static int lastBombs = -1;
    static int lastPower = -1;
    static int lastLives = -1;

    if (player.score != lastScore || player.bombs != lastBombs || player.powerLevel != lastPower || player.lives != lastLives) {
        tft.fillRect(0, 0, SCREEN_WIDTH, 20, ILI9341_BLACK);
        tft.setCursor(5, 5);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(1);
        tft.print("S:"); tft.print(player.score);
        tft.print(" B:"); tft.print(player.bombs);
        tft.print(" L:"); tft.print(player.lives);
        tft.print(" P:"); tft.print(player.powerLevel);

        lastScore = player.score;
        lastBombs = player.bombs;
        lastPower = player.powerLevel;
        lastLives = player.lives;
    }
}

void draw() {
    static float oldPlayerX = -1, oldPlayerY = -1;
    static bool oldPlayerAlive = false;
    static Bullet oldBullets[MAX_PLAYER_BULLETS];
    static Enemy oldEnemies[MAX_ENEMIES];
    static PowerUp oldPowerUps[MAX_POWERUPS];

    // Player Rendering (Blink if invulnerable)
    bool invulnerable = millis() < invulnEndTime;
    bool shouldDrawPlayer = player.alive && (!invulnerable || (millis() / 100 % 2 == 0));

    if (player.x != oldPlayerX || player.y != oldPlayerY || player.alive != oldPlayerAlive || invulnerable) {
        tft.fillRect(oldPlayerX, oldPlayerY, player.width, player.height, ILI9341_BLACK);
        if (shouldDrawPlayer) {
            tft.fillRect(player.x, player.y, player.width, player.height, ILI9341_BLUE);
        }
        oldPlayerX = player.x; oldPlayerY = player.y; oldPlayerAlive = player.alive;
    }

    // Bullets Rendering
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (bullets[i].x != oldBullets[i].x || bullets[i].y != oldBullets[i].y || bullets[i].active != oldBullets[i].active) {
            if (oldBullets[i].active) {
                tft.fillRect(oldBullets[i].x, oldBullets[i].y, oldBullets[i].width, oldBullets[i].height, ILI9341_BLACK);
            }
            if (bullets[i].active) {
                tft.fillRect(bullets[i].x, bullets[i].y, bullets[i].width, bullets[i].height, ILI9341_YELLOW);
            }
            oldBullets[i] = bullets[i];
        }
    }

    // Enemies Rendering
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].x != oldEnemies[i].x || enemies[i].y != oldEnemies[i].y || enemies[i].active != oldEnemies[i].active) {
            if (oldEnemies[i].active) {
                tft.fillRect(oldEnemies[i].x, oldEnemies[i].y, oldEnemies[i].width, oldEnemies[i].height, ILI9341_BLACK);
            }
            if (enemies[i].active) {
                uint16_t color = enemies[i].isHighLevel ? ILI9341_MAGENTA : (enemies[i].transferred ? ILI9341_RED : ILI9341_GREEN);
                tft.fillRect(enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height, color);
            }
            oldEnemies[i] = enemies[i];
        }
    }

    // Power-ups Rendering
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].x != oldPowerUps[i].x || powerups[i].y != oldPowerUps[i].y || powerups[i].active != oldPowerUps[i].active) {
            if (oldPowerUps[i].active) {
                tft.fillRect(oldPowerUps[i].x, oldPowerUps[i].y, oldPowerUps[i].width, oldPowerUps[i].height, ILI9341_BLACK);
            }
            if (powerups[i].active) {
                tft.fillRect(powerups[i].x, powerups[i].y, powerups[i].width, powerups[i].height, ILI9341_CYAN);
            }
            oldPowerUps[i] = powerups[i];
        }
    }
}

void loop() {
    if (!player.alive) {
        tft.setCursor(SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT/2);
        tft.setTextColor(ILI9341_RED); tft.setTextSize(2);
        tft.println("GAME OVER");
        return;
    }

    updatePlayer(player);

    if (digitalRead(BUTTON_SHOOT_PIN) == LOW) fireBullet(player, bullets);

    if (digitalRead(BUTTON_BOMB_PIN) == LOW && player.bombs > 0) {
        for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
        player.bombs--;
        delay(200);
    }

    updateBullets(bullets);
    updateEnemies(enemies);
    updatePowerUps();
    handleCollisions();

    if (millis() - lastSpawnTime > spawnInterval) {
        spawnEnemy(enemies, random(0, SCREEN_WIDTH - 20), false);
        lastSpawnTime = millis();
    }

    Packet p;
    if (receivePacket(p)) {
        if (p.type == TYPE_SPAWN_ENEMY) {
            float x = (p.data * SCREEN_WIDTH) / 100.0;
            spawnEnemy(enemies, x, true);
        }
    }

    draw();
    drawUI();
    delay(20);
}
