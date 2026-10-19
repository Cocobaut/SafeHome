#include "TaskDoorSensor.h"

#define DOOR_DEBOUNCE_MS 50

static bool stableOpen = false;
static bool lastRaw = false;
static unsigned long lastChange = 0;
static bool openedEvent = false;

static bool readDoorRaw()
{
    return digitalRead(PIN_DOOR) == DOOR_OPEN_LEVEL;
}

bool doorIsOpen()
{
    return stableOpen;
}

// Returns true once for every closed -> open transition
bool doorConsumeOpenedEvent()
{
    bool evt = openedEvent;
    openedEvent = false;
    return evt;
}

void loopDoorSensor()
{
    unsigned long now = millis();
    bool raw = readDoorRaw();

    if (raw != lastRaw) {
        lastRaw = raw;
        lastChange = now;
    } else if (raw != stableOpen && now - lastChange >= DOOR_DEBOUNCE_MS) {
        stableOpen = raw;
        if (stableOpen) openedEvent = true;
        Serial.println(stableOpen ? "[Door] Cua MO" : "[Door] Cua DONG");
    }
}

void initDoorSensor()
{
    pinMode(PIN_DOOR, INPUT_PULLUP);
    stableOpen = readDoorRaw();
    lastRaw = stableOpen;
    lastChange = millis();
}
