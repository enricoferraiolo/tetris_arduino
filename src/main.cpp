#include <Arduino.h>
#include <IRremote.h>
#include <LedControl.h>
#include <LiquidCrystal.h>

#define PRODUCTION false
#define MAX7219_MAX_DEVICES 1

// Configurazione LCD
LiquidCrystal lcd(13, 9, 6, 5, 7, 4); // RS, E, D4, D5, D6, D7

// Configurazione IR Receiver
const int receiverPin = 3;
IRrecv irrecv(receiverPin);
uint32_t last_decodedRawData = 0;

// Configurazione LED Matrix
LedControl lc = LedControl(12, 11, 10, 1); // DIN, CLK, CS, numero di dispositivi
bool matrixOn = false;

// Mappatura tasti IR
struct IRCode
{
  uint32_t code;
  const char *name;
};

// Mappatura TESTING (Codici Wokwi)
const IRCode TESTING_CODES[] = {
    {0x5DA2FF00, "POWER"},
    {0x1DE2FF00, "MENU"},
    {0xDD22FF00, "TEST"},
    {0xFD02FF00, "VOL+"},
    {0x3DC2FF00, "BACK/DELETE"},
    {0x1FE0FF00, "FAST BACK"},
    {0x57A8FF00, "PAUSE"},
    {0x6F90FF00, "FAST FORWARD"},
    {0x6798FF00, "VOL-"},
    {0x4FB0FF00, "C/CLEAR"},
    {0x9768FF00, "0"},
    {0xCF30FF00, "1"},
    {0xE718FF00, "2"},
    {0x857AFF00, "3"},
    {0xEF10FF00, "4"},
    {0xC738FF00, "5"},
    {0xA55AFF00, "6"},
    {0xBD42FF00, "7"},
    {0xB54AFF00, "8"},
    {0xAD52FF00, "9"}};

// Mappatura PRODUCTION (Codici reali)
const IRCode PRODUCTION_CODES[] = {
    {0xBA45FF00, "POWER"},
    {0xB847FF00, "FUNC/STOP"},
    {0xB946FF00, "VOL+"},
    {0xBB44FF00, "FAST BACK"},
    {0xBF40FF00, "PAUSE"},
    {0xBC43FF00, "FAST FORWARD"},
    {0xF807FF00, "DOWN"},
    {0xEA15FF00, "VOL-"},
    {0xF609FF00, "UP"},
    {0xE619FF00, "EQ"},
    {0xF20DFF00, "ST/REPT"},
    {0xE916FF00, "0"},
    {0xF30CFF00, "1"},
    {0xE718FF00, "2"},
    {0xA15EFF00, "3"},
    {0xF708FF00, "4"},
    {0xE31CFF00, "5"},
    {0xA55AFF00, "6"},
    {0xBD42FF00, "7"},
    {0xAD52FF00, "8"},
    {0xB54AFF00, "9"}};

const IRCode *activeMap;
size_t mapSize;

// Variabili del gioco
struct Tetromino
{
  byte shape[4];
  int width;
  int height;
};

Tetromino pieces[7] = {
    {{0b1111, 0, 0, 0}, 4, 1}, // I
    {{0b0111, 0b0100}, 3, 2},  // J
    {{0b1110, 0b0010}, 3, 2},  // L
    {{0b0110, 0b0110}, 2, 2},  // O
    {{0b0111, 0b0010}, 3, 2},  // S
    {{0b1100, 0b0110}, 3, 2},  // T
    {{0b1110, 0b1000}, 3, 2}   // Z
};

byte grid[8] = {0};
Tetromino currentPiece;
int posX, posY;
unsigned long lastFall = 0;
int fallSpeed = 1000;
bool gameActive = false;
bool gameOver = false;
bool gamePaused = false;
int score = 0;

void initializeLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.begin(16, 2);
  lcd.print("Tetris IR Remote");
  lcd.setCursor(0, 1);
  lcd.print("POWER to start");
}

void initializeActiveMap()
{
  if (PRODUCTION)
  {
    activeMap = PRODUCTION_CODES;
    mapSize = sizeof(PRODUCTION_CODES) / sizeof(PRODUCTION_CODES[0]);
  }
  else
  {
    activeMap = TESTING_CODES;
    mapSize = sizeof(TESTING_CODES) / sizeof(TESTING_CODES[0]);
  }
}

bool checkCollision(int newX, int newY)
{
  for (int y = 0; y < currentPiece.height; y++)
  {
    for (int x = 0; x < currentPiece.width; x++)
    {
      if (bitRead(currentPiece.shape[y], x))
      {
        int gX = newX + x;
        int gY = newY + y;

        if (gX < 0 || gX >= 8 || gY >= 8)
          return true;
        if (gY >= 0 && bitRead(grid[gY], gX))
          return true;
      }
    }
  }
  return false;
}

void spawnNewPiece()
{
  currentPiece = pieces[random(7)];
  posX = 3;
  posY = 0;
  if (checkCollision(posX, posY))
    gameOver = true;
}

void mergePiece()
{
  for (int y = 0; y < currentPiece.height; y++)
  {
    for (int x = 0; x < currentPiece.width; x++)
    {
      if (bitRead(currentPiece.shape[y], x))
      {
        int gX = posX + x;
        int gY = posY + y;
        if (gY >= 0 && gY < 8)
          bitSet(grid[gY], gX);
      }
    }
  }
}

void clearLines()
{
  for (int y = 0; y < 8; y++)
  {
    if (grid[y] == 0xFF)
    {
      // linea completa
      score += 10; // incrementa punteggio
      // aggiorna LCD
      lcd.setCursor(0, 1);
      lcd.print("Score: ");
      lcd.print(score);
      lcd.print("  ");
      // rialza tutte le righe sopra
      for (int yy = y; yy > 0; yy--)
        grid[yy] = grid[yy - 1];
      grid[0] = 0;
    }
  }
}

void drawMatrix()
{
  for (int row = 0; row < 8; row++)
  {
    byte output = grid[row];

    for (int y = 0; y < currentPiece.height; y++)
    {
      if (row == posY + y)
      {
        for (int x = 0; x < currentPiece.width; x++)
        {
          if (bitRead(currentPiece.shape[y], x))
          {
            int px = posX + x;
            if (px >= 0 && px < 8)
              bitSet(output, px);
          }
        }
      }
    }

    lc.setRow(0, row, output);
  }
}

void resetGame()
{
  memset(grid, 0, sizeof(grid));
  gameOver = false;
  gamePaused = false; // reset pausa
  score = 0;          // reset punteggio
  // mostra punteggio iniziale
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed: ");
  lcd.print(fallSpeed);
  lcd.setCursor(0, 1);
  lcd.print("Score: 0      ");
  spawnNewPiece();
}

void translateIR()
{
  // Debug raw code
  Serial.print("IR raw code: 0x");
  Serial.print(irrecv.decodedIRData.decodedRawData, HEX);

  uint32_t code = irrecv.decodedIRData.decodedRawData;
  const char *buttonName = "UNKNOWN";
  for (size_t i = 0; i < mapSize; i++)
  {
    if (activeMap[i].code == code)
    {
      buttonName = activeMap[i].name;
      break;
    }
  }
  Serial.print(" - Button: ");
  Serial.println(buttonName);

  if (strcmp(buttonName, "POWER") == 0)
  {
    gameActive = !gameActive;
    matrixOn = gameActive;
    if (gameActive)
    {
      resetGame();
    }
    else
    {
      lc.clearDisplay(0);
      initializeLCD();
    }
  }
  else if (strcmp(buttonName, "PAUSE") == 0)
  {
    if (gameOver)
    {
      // Restart on PAUSE when game over
      Serial.println("Restarting Game...");
      gameActive = true;
      matrixOn = true;
      resetGame();
    }
    else if (gameActive)
    {
      gamePaused = !gamePaused;
      Serial.println(gamePaused ? "Game Paused" : "Game Resumed");
      if (gamePaused)
      {
        lcd.setCursor(0, 0);
        lcd.print("Game Paused     ");
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(score);
        lc.clearDisplay(0);
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Speed: ");
        lcd.print(fallSpeed);
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(score); // mostra punteggio
        drawMatrix();
      }
    }
  }
  else if (strcmp(buttonName, "VOL+") == 0 && gameActive && !gameOver)
  {
    if (fallSpeed > 100)
      fallSpeed -= 100;
    Serial.print("Speed increased, fall interval: ");
    Serial.println(fallSpeed);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Speed: ");
    lcd.print(fallSpeed);
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score); 
  }
  else if (strcmp(buttonName, "VOL-") == 0 && gameActive && !gameOver)
  {
    if (fallSpeed < 2000)
      fallSpeed += 100;
    Serial.print("Speed decreased, fall interval: ");
    Serial.println(fallSpeed);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Speed: ");
    lcd.print(fallSpeed);
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score); 
  }
  else if (gameActive && !gameOver && !gamePaused)
  {
    if (strcmp(buttonName, "FAST BACK") == 0)
    {
      if (!checkCollision(posX - 1, posY))
        posX--;
    }
    else if (strcmp(buttonName, "FAST FORWARD") == 0)
    {
      if (!checkCollision(posX + 1, posY))
        posX++;
    }
    drawMatrix();
  }

  last_decodedRawData = code;
  delay(200);
}

void setup()
{
  initializeActiveMap();
  Serial.begin(9600);
  irrecv.enableIRIn();

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  randomSeed(analogRead(0));

  initializeLCD();
}

void loop()
{
  if (irrecv.decode())
  {
    translateIR();
    irrecv.resume();
  }

  if (gameActive && !gameOver && !gamePaused && (millis() - lastFall > fallSpeed))
  {
    if (!checkCollision(posX, posY + 1))
    {
      posY++;
    }
    else
    {
      mergePiece();
      clearLines();
      spawnNewPiece();
    }
    lastFall = millis();
    drawMatrix();
  }

  if (gameOver)
  {
    Serial.println("Game Over! Press PAUSE to restart.");
    lcd.setCursor(0, 0);
    lcd.print("Final score: ");
    lcd.print(score);
    lcd.setCursor(0, 1);
    lcd.print("PAUSE to restart");

    for (int i = 0; i < 3; i++)
    {
      lc.clearDisplay(0);
      delay(200);
      drawMatrix();
      delay(200);
    }
    gameActive = false;
  }
}
