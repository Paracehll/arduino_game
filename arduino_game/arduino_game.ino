#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Config.h"
#include "Entities.h"
#include "Communication.h"
#include "GameLogic.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

Player player;
Bullet bullets[MAX_PLAYER_BULLETS];
EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];
Enemy enemies[MAX_ENEMIES];
PowerUp powerups[MAX_POWERUPS];

unsigned long lastSpawnTime = 0;
const uint16_t spawnInterval = 2000;
unsigned long invulnEndTime = 0;

void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(ILI9341_BLACK);

    player = {SCREEN_WIDTH / 2 - 10, PLAYER_Y, 0, 3, INITIAL_LIVES, 0, 1, 20, 20};

    for (int8_t i = 0; i < MAX_PLAYER_BULLETS; i++) bullets[i].active = 0;
    for (int8_t i = 0; i < MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = 0;
    for (int8_t i = 0; i < MAX_ENEMIES; i++) enemies[i].active = 0;
    for (int8_t i = 0; i < MAX_POWERUPS; i++) powerups[i].active = 0;

    pinMode(BUTTON_SHOOT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_BOMB_PIN, INPUT_PULLUP);
    pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
}

void spawnPowerUp(float x, float y) {
    for (int8_t i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].active) {
            powerups[i].x = x;
            powerups[i].y = y;
            powerups[i].active = 1;
            powerups[i].width = 10;
            powerups[i].height = 10;
            break;
        }
    }
}

void updatePowerUps() {
    for (int8_t i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active) {
            powerups[i].y += ENEMY_SPEED;
            if (powerups[i].y > SCREEN_HEIGHT) powerups[i].active = 0;
        }
    }
}

void handleCollisions() {
    bool invulnerable = millis() < invulnEndTime;

    for (int8_t i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        if (!invulnerable && checkCollision(player.x, player.y, player.width, player.height,
                           enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height)) {
            player.lives--;
            if (player.lives <= 0) {
                player.alive = 0;
            } else {
                invulnEndTime = millis() + 2000;
                for(int8_t k=0; k<MAX_ENEMIES; k++) enemies[k].active = 0;
            }
        }

        for (int8_t j = 0; j < MAX_PLAYER_BULLETS; j++) {
            if (bullets[j].active && checkCollision(bullets[j].x, bullets[j].y, bullets[j].width, bullets[j].height,
                                                    enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height)) {
                bullets[j].active = 0;
                enemies[i].hp -= BULLET_DAMAGE;

                if (enemies[i].hp <= 0) {
                    enemies[i].active = 0;
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

    if (!invulnerable) {
        for (int8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (enemyBullets[i].active && checkCollision(player.x, player.y, player.width, player.height,
                                                        enemyBullets[i].x, enemyBullets[i].y, enemyBullets[i].width, enemyBullets[i].height)) {
                enemyBullets[i].active = 0;
                player.lives--;
                if (player.lives <= 0) {
                    player.alive = 0;
                } else {
                    invulnEndTime = millis() + 2000;
                    for(int8_t k=0; k<MAX_ENEMIES; k++) enemies[k].active = 0;
                }
                break;
            }
        }
    }

    for (int8_t i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active && checkCollision(player.x, player.y, player.width, player.height,
                                                 powerups[i].x, powerups[i].y, powerups[i].width, powerups[i].height)) {
            powerups[i].active = 0;
            player.powerLevel++;
        }
    }
}

void drawUI() {
    static int16_t lastScore = -1;
    static int8_t lastBombs = -1;
    static int8_t lastPower = -1;
    static int8_t lastLives = -1;

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

// Memory-efficient state tracking for rendering
struct RenderState { float x, y; uint8_t active; };

void draw() {
    static float oldPX = -1, oldPY = -1;
    static uint8_t oldPAlive = 0;
    static RenderState oldB[MAX_PLAYER_BULLETS];
    static RenderState oldEB[MAX_ENEMY_BULLETS];
    static RenderState oldE[MAX_ENEMIES];
    static RenderState oldP[MAX_POWERUPS];

    bool invulnerable = millis() < invulnEndTime;
    bool shouldDrawP = player.alive && (!invulnerable || (millis() / 100 % 2 == 0));

    if (player.x != oldPX || player.y != oldPY || player.alive != oldPAlive || invulnerable) {
        tft.fillRect(oldPX, oldPY, player.width, player.height, ILI9341_BLACK);
        if (shouldDrawP) tft.fillRect(player.x, player.y, player.width, player.height, ILI9341_BLUE);
        oldPX = player.x; oldPY = player.y; oldPAlive = player.alive;
    }

    for (int8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (bullets[i].x != oldB[i].x || bullets[i].y != oldB[i].y || bullets[i].active != oldB[i].active) {
            if (oldB[i].active) tft.fillRect(oldB[i].x, oldB[i].y, bullets[i].width, bullets[i].height, ILI9341_BLACK);
            if (bullets[i].active) tft.fillRect(bullets[i].x, bullets[i].y, bullets[i].width, bullets[i].height, ILI9341_YELLOW);
            oldB[i] = {bullets[i].x, bullets[i].y, bullets[i].active};
        }
    }

    for (int8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemyBullets[i].x != oldEB[i].x || enemyBullets[i].y != oldEB[i].y || enemyBullets[i].active != oldEB[i].active) {
            if (oldEB[i].active) tft.fillRect(oldEB[i].x, oldEB[i].y, enemyBullets[i].width, enemyBullets[i].height, ILI9341_BLACK);
            if (enemyBullets[i].active) tft.fillRect(enemyBullets[i].x, enemyBullets[i].y, enemyBullets[i].width, enemyBullets[i].height, ILI9341_RED);
            oldEB[i] = {enemyBullets[i].x, enemyBullets[i].y, enemyBullets[i].active};
        }
    }

    for (int8_t i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].x != oldE[i].x || enemies[i].y != oldE[i].y || enemies[i].active != oldE[i].active) {
            if (oldE[i].active) tft.fillRect(oldE[i].x, oldE[i].y, enemies[i].width, enemies[i].height, ILI9341_BLACK);
            if (enemies[i].active) {
                uint16_t color = enemies[i].isHighLevel ? ILI9341_MAGENTA : (enemies[i].transferred ? ILI9341_RED : ILI9341_GREEN);
                tft.fillRect(enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height, color);
            }
            oldE[i] = {enemies[i].x, enemies[i].y, enemies[i].active};
        }
    }

    for (int8_t i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].x != oldP[i].x || powerups[i].y != oldP[i].y || powerups[i].active != oldP[i].active) {
            if (oldP[i].active) tft.fillRect(oldP[i].x, oldP[i].y, powerups[i].width, powerups[i].height, ILI9341_BLACK);
            if (powerups[i].active) tft.fillRect(powerups[i].x, powerups[i].y, powerups[i].width, powerups[i].height, ILI9341_CYAN);
            oldP[i] = {powerups[i].x, powerups[i].y, powerups[i].active};
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
        for (int8_t i = 0; i < MAX_ENEMIES; i++) enemies[i].active = 0;
        for (int8_t i = 0; i < MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = 0;
        player.bombs--;
        delay(200);
    }

    updateBullets(bullets);
    updateEnemyBullets(enemyBullets);
    updateEnemies(enemies, player, enemyBullets);
    updatePowerUps();
    handleCollisions();

    if (millis() - lastSpawnTime > spawnInterval) {
        spawnEnemy(enemies, random(0, SCREEN_WIDTH - 20), 0);
        lastSpawnTime = millis();
    }

    Packet p;
    if (receivePacket(p)) {
        if (p.type == TYPE_SPAWN_ENEMY) {
            float x = (p.data * SCREEN_WIDTH) / 100.0;
            spawnEnemy(enemies, x, 1);
        }
    }

    draw();
    drawUI();
    delay(20);
}
