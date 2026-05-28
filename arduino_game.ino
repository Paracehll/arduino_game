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

unsigned long lastSpawnTime = 0;
const unsigned long spawnInterval = 2000;

void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setRotation(0); // Vertical portrait mode
    tft.fillScreen(ILI9341_BLACK);

    player = {SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT - 40, 0, 3, true, 20, 20};

    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        bullets[i] = {0, 0, false, 4, 10};
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }

    pinMode(BUTTON_SHOOT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_BOMB_PIN, INPUT_PULLUP);
}

void handleCollisions() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        // Player vs Enemy
        if (checkCollision(player.x, player.y, player.width, player.height,
                           enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height)) {
            player.alive = false;
        }

        // Bullet vs Enemy
        for (int j = 0; j < MAX_PLAYER_BULLETS; j++) {
            if (bullets[j].active && checkCollision(bullets[j].x, bullets[j].y, bullets[j].width, bullets[j].height,
                                                    enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height)) {
                bullets[j].active = false;
                enemies[i].active = false;
                player.score += 10;

                // Transfer logic: if it's a neutral unit (not already transferred), send it to opponent
                if (!enemies[i].transferred) {
                    uint8_t x_percent = (enemies[i].x * 100) / SCREEN_WIDTH;
                    sendEnemyTransfer(x_percent);
                }
            }
        }
    }
}

void drawUI() {
    static int lastScore = -1;
    static int lastBombs = -1;

    if (player.score != lastScore || player.bombs != lastBombs) {
        tft.fillRect(0, 0, SCREEN_WIDTH, 20, ILI9341_BLACK);
        tft.setCursor(5, 5);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(1);
        tft.print("Score: ");
        tft.print(player.score);
        tft.print("  Bombs: ");
        tft.print(player.bombs);

        lastScore = player.score;
        lastBombs = player.bombs;
    }
}

void draw() {
    static float oldPlayerX, oldPlayerY;
    static Bullet oldBullets[MAX_PLAYER_BULLETS];
    static Enemy oldEnemies[MAX_ENEMIES];

    // Simple clearing by redrawing old positions with black
    tft.fillRect(oldPlayerX, oldPlayerY, player.width, player.height, ILI9341_BLACK);
    for(int i=0; i<MAX_PLAYER_BULLETS; i++) if(oldBullets[i].active) tft.fillRect(oldBullets[i].x, oldBullets[i].y, oldBullets[i].width, oldBullets[i].height, ILI9341_BLACK);
    for(int i=0; i<MAX_ENEMIES; i++) if(oldEnemies[i].active) tft.fillRect(oldEnemies[i].x, oldEnemies[i].y, oldEnemies[i].width, oldEnemies[i].height, ILI9341_BLACK);

    if (player.alive) {
        tft.fillRect(player.x, player.y, player.width, player.height, ILI9341_BLUE);
    }

    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (bullets[i].active) {
            tft.fillRect(bullets[i].x, bullets[i].y, bullets[i].width, bullets[i].height, ILI9341_YELLOW);
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            uint16_t color = enemies[i].transferred ? ILI9341_RED : ILI9341_GREEN;
            tft.fillRect(enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height, color);
        }
    }

    // Update old positions
    oldPlayerX = player.x; oldPlayerY = player.y;
    for(int i=0; i<MAX_PLAYER_BULLETS; i++) oldBullets[i] = bullets[i];
    for(int i=0; i<MAX_ENEMIES; i++) oldEnemies[i] = enemies[i];
}

void loop() {
    if (!player.alive) {
        tft.setCursor(SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT/2);
        tft.setTextColor(ILI9341_RED);
        tft.setTextSize(2);
        tft.println("GAME OVER");
        return;
    }

    updatePlayer(player);

    if (digitalRead(BUTTON_SHOOT_PIN) == LOW) { // Active Low
        fireBullet(player, bullets);
    }

    if (digitalRead(BUTTON_BOMB_PIN) == LOW && player.bombs > 0) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            enemies[i].active = false;
        }
        player.bombs--;
        delay(200); // Debounce/Limit bomb rate
    }

    updateBullets(bullets);
    updateEnemies(enemies);
    handleCollisions();

    // Spawn Neutral Units
    if (millis() - lastSpawnTime > spawnInterval) {
        spawnEnemy(enemies, random(0, SCREEN_WIDTH - 20), false);
        lastSpawnTime = millis();
    }

    // Receive Pressure Units
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
