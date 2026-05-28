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

void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(ILI9341_BLACK);

    player = {SCREEN_WIDTH / 2 - 10, PLAYER_Y, 0, 3, 0, true, 20, 20};

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
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        if (checkCollision(player.x, player.y, player.width, player.height,
                           enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height)) {
            player.alive = false;
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

    if (player.score != lastScore || player.bombs != lastBombs || player.powerLevel != lastPower) {
        tft.fillRect(0, 0, SCREEN_WIDTH, 20, ILI9341_BLACK);
        tft.setCursor(5, 5);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(1);
        tft.print("Score:"); tft.print(player.score);
        tft.print(" B:"); tft.print(player.bombs);
        tft.print(" P:"); tft.print(player.powerLevel);

        lastScore = player.score;
        lastBombs = player.bombs;
        lastPower = player.powerLevel;
    }
}

void draw() {
    static float oldPlayerX, oldPlayerY;
    static Bullet oldBullets[MAX_PLAYER_BULLETS];
    static Enemy oldEnemies[MAX_ENEMIES];
    static PowerUp oldPowerUps[MAX_POWERUPS];

    tft.fillRect(oldPlayerX, oldPlayerY, player.width, player.height, ILI9341_BLACK);
    for(int i=0; i<MAX_PLAYER_BULLETS; i++) if(oldBullets[i].active) tft.fillRect(oldBullets[i].x, oldBullets[i].y, oldBullets[i].width, oldBullets[i].height, ILI9341_BLACK);
    for(int i=0; i<MAX_ENEMIES; i++) if(oldEnemies[i].active) tft.fillRect(oldEnemies[i].x, oldEnemies[i].y, oldEnemies[i].width, oldEnemies[i].height, ILI9341_BLACK);
    for(int i=0; i<MAX_POWERUPS; i++) if(oldPowerUps[i].active) tft.fillRect(oldPowerUps[i].x, oldPowerUps[i].y, oldPowerUps[i].width, oldPowerUps[i].height, ILI9341_BLACK);

    if (player.alive) tft.fillRect(player.x, player.y, player.width, player.height, ILI9341_BLUE);

    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (bullets[i].active) tft.fillRect(bullets[i].x, bullets[i].y, bullets[i].width, bullets[i].height, ILI9341_YELLOW);
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            uint16_t color = enemies[i].isHighLevel ? ILI9341_MAGENTA : (enemies[i].transferred ? ILI9341_RED : ILI9341_GREEN);
            tft.fillRect(enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height, color);
        }
    }

    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active) tft.fillRect(powerups[i].x, powerups[i].y, powerups[i].width, powerups[i].height, ILI9341_CYAN);
    }

    oldPlayerX = player.x; oldPlayerY = player.y;
    for(int i=0; i<MAX_PLAYER_BULLETS; i++) oldBullets[i] = bullets[i];
    for(int i=0; i<MAX_ENEMIES; i++) oldEnemies[i] = enemies[i];
    for(int i=0; i<MAX_POWERUPS; i++) oldPowerUps[i] = powerups[i];
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
