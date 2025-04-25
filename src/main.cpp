#include <Arduino.h>
#include <IRremote.h>
#include <LedControl.h>

#define PRODUCTION false
#define MAX7219_MAX_DEVICES 1

// IR receiver on digital pin 3
const int receiverPin = 3;
IRrecv irrecv(receiverPin);
uint32_t last_decodedRawData = 0;

// LED matrix: DIN=D12, CLK=D11, CS=D10, and 4 devices in chain
LedControl lc = LedControl(/*DIN=*/12, /*CLK=*/11, /*CS=*/10, /*#devices=*/4);
bool matrixOn = false;

// IR code → name mapping for Wokwi TESTING remote
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

void setup()
{
  initializeActiveMap();
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode");
  irrecv.enableIRIn();
  Serial.println("IR Receiver Button Decode");
  irrecv.enableIRIn();

  // Initialize each MAX7219 in the chain
  for (int dev = 0; dev < MAX7219_MAX_DEVICES; ++dev)
  {
    lc.shutdown(dev, false); // Wake up
    lc.setIntensity(dev, 8); // Mid brightness (0–15)
    lc.clearDisplay(dev);    // All LEDs off
  }
}

void toggleMatrix(bool on)
{
  // For each of the devices:
  for (int dev = 0; dev < MAX7219_MAX_DEVICES; ++dev)
  {
    if (on)
    {
      // Light every LED: rows 0–7, cols 0–7
      for (int row = 0; row < 8; ++row)
      {
        lc.setRow(dev, row, 0xFF);
      }
    }
    else
    {
      // Turn all off
      lc.clearDisplay(dev);
    }
  }
}

void translateIR()
{
  // Handle REPEAT flag by reusing last_decodedRawData
  if (irrecv.decodedIRData.flags)
  {
    irrecv.decodedIRData.decodedRawData = last_decodedRawData;
    Serial.println("REPEAT!");
  }
  else
  {
    Serial.print("IR code: 0x");
    Serial.println(irrecv.decodedIRData.decodedRawData, HEX);
  }

  uint32_t code = irrecv.decodedIRData.decodedRawData;
  bool found = false;
  const char *buttonName = nullptr;

  // Look up the button name
  for (size_t i = 0; i < mapSize; i++)
  {
    if (activeMap[i].code == code)
    {
      buttonName = activeMap[i].name;
      found = true;
      break;
    }
  }

  if (found)
  {
    Serial.println(buttonName);
    // If POWER pressed, toggle the matrix
    if (strcmp(buttonName, "POWER") == 0)
    {
      matrixOn = !matrixOn;
      toggleMatrix(matrixOn);
      Serial.print("Matrix is now ");
      Serial.println(matrixOn ? "ON" : "OFF");
    }
  }
  else
  {
    Serial.println("other button");
  }

  last_decodedRawData = code;
  delay(200); // debounce
}

void loop()
{
  if (irrecv.decode())
  {
    translateIR();
    irrecv.resume();
  }
}
