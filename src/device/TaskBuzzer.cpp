#include "TaskBuzzer.h"

#define BUZZER_OFF_LEVEL (BUZZER_ON_LEVEL == HIGH ? LOW : HIGH)
#define BUZZER_BEEP_MS   250

static BuzzerMode buzzerMode = BUZZER_OFF;
static bool outputOn = false;
static unsigned long lastToggle = 0;

static void buzzerOutput(bool on)
{
    outputOn = on;
    digitalWrite(PIN_BUZZER, on ? BUZZER_ON_LEVEL : BUZZER_OFF_LEVEL);
}

void buzzerSetMode(BuzzerMode mode)
{
    if (mode == buzzerMode) return;
    buzzerMode = mode;
    lastToggle = millis();
    buzzerOutput(mode != BUZZER_OFF);
}

bool buzzerIsOn()
{
    return buzzerMode != BUZZER_OFF;
}

void loopBuzzer()
{
    if (buzzerMode != BUZZER_BEEP) return;

    unsigned long now = millis();
    if (now - lastToggle >= BUZZER_BEEP_MS) {
        lastToggle = now;
        buzzerOutput(!outputOn);
    }
}

void initBuzzer()
{
    pinMode(PIN_BUZZER, OUTPUT);
    buzzerOutput(false);
}
