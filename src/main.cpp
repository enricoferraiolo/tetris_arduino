#include <utils.h>
#include <Arduino.h>
#include <IRremote.h>
#include <LedControl.h>
#include <LiquidCrystal.h>

// Costanti
#define PRODUCTION false
#define MAX7219_MAX_DEVICES 1 // Numero massimo di dispositivi MAX7219
#define RECEIVER_PIN 3        // Pin per il ricevitore IR
// Configurazione pin LCD
#define LCD_RS_PIN 13
#define LCD_E_PIN 9
#define LCD_D4_PIN 6
#define LCD_D5_PIN 5
#define LCD_D6_PIN 7
#define LCD_D7_PIN 4
// Configurazione pin LED matrix
#define MATRIX_DIN_PIN 12
#define MATRIX_CLK_PIN 11
#define MATRIX_CS_PIN 10
// Configurazione pin rotary encoder
#define ENCODER_CLK_PIN 2
#define ENCODER_DT_PIN 8
#define ENCODER_SW_PIN -1 // Set to actual pin if using a button, -1 if not used
// Configurazione del gioco
#define MIN_FALL_SPEED 100
#define MAX_FALL_SPEED 2000
#define SPEED_INCREMENT 100
#define POINTS_PER_LINE 10    // Punti per linea completata
#define BLINK_COUNT 3         // Numero di lampeggiamenti in caso di game over
#define BLINK_DELAY 200       // Millisecondi di ritardo tra i lampeggiamenti
#define IR_DEBOUNCE_DELAY 200 // Millisecondi di debounce per il ricevitore IR
#define ENCODER_DEBOUNCE 50   // Millisecondi di debounce per l'encoder (aumentato)

// Configurazione LCD
LiquidCrystal lcd(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

// Configurazione IR Receiver
IRrecv irRecv(RECEIVER_PIN);
uint32_t last_decodedRawData = 0;

// Configurazione LED Matrix
LedControl ledMatrix = LedControl(MATRIX_DIN_PIN, MATRIX_CLK_PIN, MATRIX_CS_PIN, MAX7219_MAX_DEVICES);
bool matrixOn = false;

const IRCode *activeMap;
size_t mapSize;

// Configurazione rotary encoder
volatile int lastEncoderState;
volatile unsigned long lastEncoderTime = 0;
volatile bool encoderChanged = false;
volatile int encoderDirection = 0; // 1: clockwise, -1: counter-clockwise, 0: no change

// Definizione della struttura Tetromino
struct Tetromino
{
  byte shape[4];
  int width;
  int height;
};

// Definizione delle forme dei tetramini
const int TETROMINO_SHAPES_COUNT = 7;
const Tetromino TETROMINO_SHAPES[TETROMINO_SHAPES_COUNT] = {
    // Ogni bit a 1 rappresenta un LED accesso sulla riga x
    {{0b1111, 0, 0, 0}, 4, 1},      // I
    {{0b0111, 0b0100, 0, 0}, 3, 2}, // J
    {{0b0111, 0b0001, 0, 0}, 3, 2}, // L
    {{0b0011, 0b0011, 0, 0}, 2, 2}, // O
    {{0b0111, 0b0010, 0, 0}, 3, 2}, // S
    {{0b0110, 0b0011, 0, 0}, 3, 2}, // T
    {{0b0011, 0b0110, 0, 0}, 3, 2}  // Z
};

// Variabili di gioco
byte grid[8] = {0};
Tetromino currentPiece;
int posX, posY;
unsigned long lastFall = 0;
int fallSpeed = 1000;
bool gameActive = false;
bool gameOver = false;
bool gamePaused = false;
int score = 0;

// Prototipi
void initializeLCD();
void initializeActiveMap();
void initializeEncoder();
bool checkCollision(int newX, int newY);
void spawnNewPiece();
void mergePiece();
void clearLines();
void drawMatrix();
void resetGame();
void translateIR();
void handleGameOver();
void updateSpeedDisplay();
void toggleGamePause();
void toggleGameActive();
void handleEncoderChange();
void encoderISR();

void setup()
{
  Serial.begin(9600);

  // Inizializzazioni
  initializeActiveMap();
  irRecv.enableIRIn();

  // Inizializzazione della matrice LED
  ledMatrix.shutdown(0, false);
  ledMatrix.setIntensity(0, 8);
  ledMatrix.clearDisplay(0);

  // Inizializzazione random
  randomSeed(analogRead(0));

  // Inizializzazione del display LCD
  lcd.begin(16, 2);
  initializeLCD();

  // Inizializzazione rotary encoder
  initializeEncoder();
}

void loop()
{
  // Gestione dell'input IR
  if (irRecv.decode())
  {
    translateIR();
    irRecv.resume();
  }

  // Gestione dell'encoder rotativo
  if (encoderChanged && gameActive && !gameOver && !gamePaused)
  {
    handleEncoderChange();
    encoderChanged = false;
  }

  // Logica di caduta del pezzo
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

  // Gestione del game over
  if (gameOver)
  {
    handleGameOver();
  }
}

void initializeEncoder()
{
  pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DT_PIN, INPUT_PULLUP);

  lastEncoderState = digitalRead(ENCODER_CLK_PIN);

  // Configura interrupt solo sul fronte di SALITA del pin CLK dell'encoder
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), encoderISR, RISING);
}

void encoderISR()
{
  // Debounce
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderTime < ENCODER_DEBOUNCE)
  {
    return;
  }

  // Lettura dei pin quando CLK è HIGH (siamo sicuri che è al livello alto grazie all'interrupt RISING)
  // Se DT è basso quando CLK è alto, rotazione oraria
  // Se DT è alto quando CLK è alto, rotazione antioraria
  if (digitalRead(ENCODER_DT_PIN) == LOW)
  {
    encoderDirection = 1; // Clockwise (orario)
  }
  else
  {
    encoderDirection = -1; // Counter-clockwise (antiorario)
  }

  encoderChanged = true;
  lastEncoderTime = currentTime;
}

void handleEncoderChange()
{
  if (encoderDirection == 1)
  {
    // Rotazione in senso orario - aumenta velocità
    if (fallSpeed > MIN_FALL_SPEED)
    {
      fallSpeed -= SPEED_INCREMENT;
      updateSpeedDisplay();
    }
  }
  else if (encoderDirection == -1)
  {
    // Rotazione in senso antiorario - diminuisce velocità
    if (fallSpeed < MAX_FALL_SPEED)
    {
      fallSpeed += SPEED_INCREMENT;
      updateSpeedDisplay();
    }
  }

  encoderDirection = 0;
}

void initializeLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
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

        // Verifica collisioni con i bordi e i pezzi già presenti
        if (gX < 0 || gX >= 8 || gY >= 8)
        {
          return true;
        }
        if (gY >= 0 && bitRead(grid[gY], gX))
        {
          return true;
        }
      }
    }
  }
  return false;
}

void spawnNewPiece()
{
  currentPiece = TETROMINO_SHAPES[random(TETROMINO_SHAPES_COUNT)];
  posX = 3;
  posY = 0;

  // Verifica se il nuovo pezzo può essere posizionato (game over check)
  if (checkCollision(posX, posY))
  {
    gameOver = true;
  }
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
        {
          bitSet(grid[gY], gX);
        }
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
      // Linea completa
      score += POINTS_PER_LINE;

      // Aggiorna schermo LCD con il punteggio
      lcd.setCursor(0, 1);
      lcd.print("Score: ");
      lcd.print(score);
      lcd.print("  ");

      // Rialza tutte le righe sopra
      for (int yy = y; yy > 0; yy--)
      {
        grid[yy] = grid[yy - 1];
      }
      grid[0] = 0;
    }
  }
}

void drawMatrix()
{
  for (int row = 0; row < 8; row++)
  {
    byte output = grid[row];

    // Aggiungi il pezzo corrente alla visualizzazione
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
            {
              bitSet(output, px);
            }
          }
        }
      }
    }

    ledMatrix.setRow(0, row, output);
  }
}

void resetGame()
{
  memset(grid, 0, sizeof(grid));
  gameOver = false;
  gamePaused = false;
  score = 0;

  // Mostra punteggio iniziale
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
  Serial.print(irRecv.decodedIRData.decodedRawData, HEX);

  uint32_t code = irRecv.decodedIRData.decodedRawData;
  const char *buttonName = "UNKNOWN";

  // Trova il nome del pulsante
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

  // Gestione dei tasti
  if (strcmp(buttonName, "POWER") == 0)
  {
    toggleGameActive();
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
      toggleGamePause();
    }
  }
  else if (gameActive && !gameOver && !gamePaused)
  {
    if (strcmp(buttonName, "VOL+") == 0)
    {
      // Aumenta velocità
      if (fallSpeed > MIN_FALL_SPEED)
      {
        fallSpeed -= SPEED_INCREMENT;
      }
      updateSpeedDisplay();
    }
    else if (strcmp(buttonName, "VOL-") == 0)
    {
      // Diminuisci velocità
      if (fallSpeed < MAX_FALL_SPEED)
      {
        fallSpeed += SPEED_INCREMENT;
      }
      updateSpeedDisplay();
    }
    else if (strcmp(buttonName, "FAST BACK") == 0)
    {
      // Sposta a sinistra
      if (!checkCollision(posX - 1, posY))
      {
        Serial.println("SPOSTO A SINISTRA");
        posX--;
      }
      drawMatrix();
    }
    else if (strcmp(buttonName, "FAST FORWARD") == 0)
    {
      // Sposta a destra
      if (!checkCollision(posX + 1, posY))
      {
        Serial.println("SPOSTO A DESTRA");
        posX++;
      }
      drawMatrix();
    }
  }

  last_decodedRawData = code;
  delay(IR_DEBOUNCE_DELAY);
}

void handleGameOver()
{
  Serial.println("Game Over! Press PAUSE to restart.");

  lcd.setCursor(0, 0);
  lcd.print("Final score: ");
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print("PAUSE to restart");

  // Lampeggia per indicare game over
  for (int i = 0; i < BLINK_COUNT; i++)
  {
    ledMatrix.clearDisplay(0);
    delay(BLINK_DELAY);
    drawMatrix();
    delay(BLINK_DELAY);
  }

  gameActive = false;
}

void updateSpeedDisplay()
{
  Serial.print("Speed changed, fall interval: ");
  Serial.println(fallSpeed);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed: ");
  lcd.print(fallSpeed);
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
}

void toggleGamePause()
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
    ledMatrix.clearDisplay(0);
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Speed: ");
    lcd.print(fallSpeed);
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
    drawMatrix();
  }
}

void toggleGameActive()
{
  gameActive = !gameActive;
  matrixOn = gameActive;

  if (gameActive)
  {
    resetGame();
  }
  else
  {
    ledMatrix.clearDisplay(0);
    initializeLCD();
  }
}