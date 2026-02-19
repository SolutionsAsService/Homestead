#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

bool grid[8][8];

// ===== Adjustable Parameters =====
#define SPAWN_CHANCE 40     // Lower = more sand (1 in X chance per frame)
#define FRAME_DELAY 60      // Lower = faster animation
#define WIND_CHANCE 200     // 0 = no wind, higher = rarer gusts

int windDirection = 0;      // -1 = left, 1 = right, 0 = none
unsigned long lastWindTime = 0;

void setup() {
  matrix.begin(0x70);
  randomSeed(analogRead(A0));
}

void loop() {
  maybeChangeWind();
  spawnSand();
  updatePhysics();
  drawGrid();
  delay(FRAME_DELAY);
}

// ----------------------------
// Spawn new sand at top
// ----------------------------
void spawnSand() {
  if (random(SPAWN_CHANCE) == 0) {
    int col = random(8);
    if (!grid[0][col]) {
      grid[0][col] = true;
    }
  }
}

// ----------------------------
// Sand physics
// ----------------------------
void updatePhysics() {

  // Bottom to top so particles don’t move twice
  for (int row = 6; row >= 0; row--) {
    for (int col = 0; col < 8; col++) {

      if (!grid[row][col]) continue;

      int newCol = col + windDirection;

      // Try straight down
      if (!grid[row + 1][col]) {
        moveSand(row, col, row + 1, col);
      }

      // Randomize diagonal direction for realism
      else {
        bool tryLeftFirst = random(2);

        if (tryLeftFirst) {
          tryDiagonal(row, col, -1);
          tryDiagonal(row, col, 1);
        } else {
          tryDiagonal(row, col, 1);
          tryDiagonal(row, col, -1);
        }
      }
    }
  }
}

// ----------------------------
// Try diagonal movement
// ----------------------------
void tryDiagonal(int row, int col, int dir) {
  int newCol = col + dir;

  if (newCol >= 0 && newCol < 8) {
    if (!grid[row + 1][newCol]) {
      moveSand(row, col, row + 1, newCol);
    }
  }
}

// ----------------------------
// Move particle
// ----------------------------
void moveSand(int fromRow, int fromCol, int toRow, int toCol) {
  grid[fromRow][fromCol] = false;
  grid[toRow][toCol] = true;
}

// ----------------------------
// Optional wind effect
// ----------------------------
void maybeChangeWind() {
  if (random(WIND_CHANCE) == 0) {
    windDirection = random(-1, 2);  // -1, 0, or 1
  }
}

// ----------------------------
// Draw matrix
// ----------------------------
void drawGrid() {
  matrix.clear();

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (grid[row][col]) {
        matrix.drawPixel(col, row, LED_ON);
      }
    }
  }

  matrix.writeDisplay();
}
