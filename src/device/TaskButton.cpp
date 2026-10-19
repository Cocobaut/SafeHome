#include "TaskButton.h"

#define BUTTON_DEBOUNCE_MS   30
#define BUTTON_LONG_PRESS_MS 3000

struct Button {
    uint8_t pin;
    bool lastRaw;
    bool pressed;
    unsigned long lastChange;
    unsigned long pressedAt;
    bool longFired;
    bool shortEvent;
    bool longEvent;
};

static Button buttons[BUTTON_COUNT] = {
    {PIN_BUTTON_1},
};

bool buttonConsumeShortPress(uint8_t index)
{
    if (index >= BUTTON_COUNT) return false;
    bool evt = buttons[index].shortEvent;
    buttons[index].shortEvent = false;
    return evt;
}

bool buttonConsumeLongPress(uint8_t index)
{
    if (index >= BUTTON_COUNT) return false;
    bool evt = buttons[index].longEvent;
    buttons[index].longEvent = false;
    return evt;
}

void loopButtons()
{
    unsigned long now = millis();

    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        Button &b = buttons[i];
        bool raw = digitalRead(b.pin) == LOW;

        if (raw != b.lastRaw) {
            b.lastRaw = raw;
            b.lastChange = now;
        } else if (raw != b.pressed && now - b.lastChange >= BUTTON_DEBOUNCE_MS) {
            b.pressed = raw;
            if (b.pressed) {
                b.pressedAt = now;
                b.longFired = false;
            } else if (!b.longFired) {
                b.shortEvent = true;
            }
        }

        if (b.pressed && !b.longFired && now - b.pressedAt >= BUTTON_LONG_PRESS_MS) {
            b.longFired = true;
            b.longEvent = true;
        }
    }
}

void initButtons()
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        pinMode(buttons[i].pin, INPUT_PULLUP);
        buttons[i].lastRaw = digitalRead(buttons[i].pin) == LOW;
        buttons[i].pressed = buttons[i].lastRaw;
        buttons[i].longFired = true;  // A button held during boot fires nothing
    }
}
