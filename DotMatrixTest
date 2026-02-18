#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

bool grid[8][8];

void setup() {
  matrix.begin(0x70); // Default I2C address
  randomSeed(analogRead(0));
}

void loop() {
  spawnSand();
  updatePhysics();
  drawGrid();
  delay(150);
}

void spawnSand() {
  if (random(0, 3) == 0) {
    int col = random(0, 8);
    if (!grid[0][col]) {
      grid[0][col] = true;
    }
  }
}

void updatePhysics() {
  for (int row = 6; row >= 0; row--) {
    for (int col = 0; col < 8; col++) {
      if (grid[row][col]) {

        // Try down
        if (!grid[row + 1][col]) {
          grid[row][col] = false;
          grid[row + 1][col] = true;
        }
        // Try down-left
        else if (col > 0 && !grid[row + 1][col - 1]) {
          grid[row][col] = false;
          grid[row + 1][col - 1] = true;
        }
        // Try down-right
        else if (col < 7 && !grid[row + 1][col + 1]) {
          grid[row][col] = false;
          grid[row + 1][col + 1] = true;
        }
      }
    }
  }
}

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
