#include "TaskRelay.h"

#define RELAY_OFF_LEVEL (RELAY_ON_LEVEL == LOW ? HIGH : LOW)

static bool relayOn = false;

void relayWrite(bool on)
{
    if (on == relayOn) return;
    relayOn = on;
    digitalWrite(PIN_RELAY, on ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL);
    Serial.println(on ? "[Relay] Motor xit cay: ON" : "[Relay] Motor xit cay: OFF");
}

bool relayIsOn()
{
    return relayOn;
}

void initRelay()
{
    // Write the OFF level before switching to output to avoid a glitch on boot
    digitalWrite(PIN_RELAY, RELAY_OFF_LEVEL);
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, RELAY_OFF_LEVEL);
    relayOn = false;
}
