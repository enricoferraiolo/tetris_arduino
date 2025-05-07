#include <Arduino.h>

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
    {0xBB44FF00, "FAST FORWARD"},
    {0xBF40FF00, "PAUSE"},
    {0xBC43FF00, "FAST BACK"},
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
